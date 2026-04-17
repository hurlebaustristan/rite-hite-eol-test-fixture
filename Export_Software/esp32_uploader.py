from __future__ import annotations

import shlex
import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Callable

from app_paths import get_bundle_dir
from serial_client import PortChoice, list_available_ports
from translations import tr


TEST_BOARD_NAME = "ESP32S3 Dev Module"
ESP32_CHIP = "esp32s3"
ESPTOOL_BUNDLED_PATH = get_bundle_dir() / "tools" / "esptool.exe"
TEST_FIRMWARE_DIR = get_bundle_dir() / "esp32_test_firmware"
FLASH_ARGS_FILE = "flash_args"
REQUIRED_FIRMWARE_FILES = (
    "EOL_RiteHite_Final.ino.bin",
    "EOL_RiteHite_Final.ino.bootloader.bin",
    "EOL_RiteHite_Final.ino.partitions.bin",
    "boot_app0.bin",
    FLASH_ARGS_FILE,
)

STLINK_MARKERS = (
    "st-link",
    "stlink",
    "stmicroelectronics",
    "stm32 stlink",
    "virtual com port",
)


@dataclass(frozen=True)
class Esp32UploadConfig:
    flasher_path: Path
    board_name: str
    firmware_dir: Path


@dataclass(frozen=True)
class Esp32UploadChoice:
    upload_port: str
    label: str
    pair_ports: tuple[str, ...]
    is_pair: bool


@dataclass(frozen=True)
class Esp32UploadDiscovery:
    auto_choice: Esp32UploadChoice | None
    pair_choices: list[Esp32UploadChoice]
    manual_choices: list[Esp32UploadChoice]
    reason: str | None


class Esp32UploadError(Exception):
    pass


class FlasherToolNotFoundError(Esp32UploadError):
    pass


class FirmwareBundleNotFoundError(Esp32UploadError):
    pass


class UploadCommandError(Esp32UploadError):
    def __init__(self, stage_name: str, return_code: int) -> None:
        super().__init__(f"{stage_name} failed. ESP32 flasher exited with code {return_code}.")
        self.stage_name = stage_name
        self.return_code = return_code


def get_default_upload_config() -> Esp32UploadConfig:
    return Esp32UploadConfig(
        flasher_path=_resolve_flasher_path(),
        board_name=TEST_BOARD_NAME,
        firmware_dir=_resolve_firmware_bundle_path(),
    )


def discover_upload_targets() -> Esp32UploadDiscovery:
    ports = [port for port in list_available_ports() if not _looks_like_stlink(port)]
    ports_by_number: list[tuple[int, PortChoice]] = []
    for port in ports:
        number = _extract_com_number(port.device)
        if number is None:
            continue
        ports_by_number.append((number, port))

    ports_by_number.sort(key=lambda item: item[0])

    pair_choices: list[Esp32UploadChoice] = []
    seen_pairs: set[tuple[str, str]] = set()
    for index in range(len(ports_by_number) - 1):
        left_number, left_port = ports_by_number[index]
        right_number, right_port = ports_by_number[index + 1]
        if right_number - left_number != 1:
            continue

        pair_key = (left_port.device, right_port.device)
        if pair_key in seen_pairs:
            continue
        seen_pairs.add(pair_key)

        pair_choices.append(
            Esp32UploadChoice(
                upload_port=left_port.device,
                label=f"{left_port.device} / {right_port.device} -> upload on {left_port.device}",
                pair_ports=(left_port.device, right_port.device),
                is_pair=True,
            )
        )

    manual_choices = list(pair_choices)
    manual_choices.extend(
        Esp32UploadChoice(
            upload_port=port.device,
            label=f"{port.device} -> manual direct upload",
            pair_ports=(port.device,),
            is_pair=False,
        )
        for _, port in ports_by_number
    )

    if len(pair_choices) == 1:
        return Esp32UploadDiscovery(
            auto_choice=pair_choices[0],
            pair_choices=pair_choices,
            manual_choices=manual_choices,
            reason=None,
        )

    if len(pair_choices) == 0:
        return Esp32UploadDiscovery(
            auto_choice=None,
            pair_choices=pair_choices,
            manual_choices=manual_choices,
            reason=tr("esp_no_pair_reason"),
        )

    return Esp32UploadDiscovery(
        auto_choice=None,
        pair_choices=pair_choices,
        manual_choices=manual_choices,
        reason=tr("esp_multi_pair_reason"),
    )


def describe_esp_prog_detection() -> tuple[bool, str]:
    discovery = discover_upload_targets()
    if discovery.auto_choice is not None:
        return (True, tr("esp_detected_on", port=discovery.auto_choice.upload_port))
    if discovery.pair_choices:
        return (True, tr("esp_multiple_candidates", count=len(discovery.pair_choices)))
    if discovery.manual_choices:
        return (False, tr("esp_not_detected"))
    return (False, tr("esp_not_detected"))


def run_upload(
    config: Esp32UploadConfig,
    selection: Esp32UploadChoice,
    log_callback: Callable[[str], None],
) -> None:
    upload_command = _build_upload_command(config, selection)
    log_callback(f"Using ESP32 flasher: {config.flasher_path}")
    log_callback(f"Using prebuilt firmware bundle: {config.firmware_dir}")
    log_callback(f"Uploading test firmware to {selection.upload_port}...")
    _run_command(upload_command, log_callback, "Upload")
    log_callback(f"Upload succeeded on {selection.upload_port}.")


def _build_upload_command(config: Esp32UploadConfig, selection: Esp32UploadChoice) -> list[str]:
    flash_options, flash_segments = _read_flash_args(config.firmware_dir)
    command = [
        str(config.flasher_path),
        "--chip",
        ESP32_CHIP,
        "--port",
        selection.upload_port,
        "--baud",
        "921600",
        "--before",
        "default_reset",
        "--after",
        "hard_reset",
        "write_flash",
        "-z",
    ]
    command.extend(flash_options)
    command.extend(flash_segments)
    return command


def _read_flash_args(firmware_dir: Path) -> tuple[list[str], list[str]]:
    flash_args_path = firmware_dir / FLASH_ARGS_FILE
    if not flash_args_path.exists():
        raise FirmwareBundleNotFoundError(f"ESP32 flash arguments were not found: {flash_args_path}")

    raw_lines = [line.strip() for line in flash_args_path.read_text(encoding="utf-8").splitlines() if line.strip()]
    if not raw_lines:
        raise FirmwareBundleNotFoundError(f"ESP32 flash arguments file is empty: {flash_args_path}")

    flash_options = shlex.split(raw_lines[0])
    flash_segments: list[str] = []

    for line in raw_lines[1:]:
        parts = shlex.split(line)
        if len(parts) != 2:
            raise FirmwareBundleNotFoundError(f"Invalid flash args entry in {flash_args_path}: {line}")

        offset, relative_name = parts
        binary_path = firmware_dir / relative_name
        if not binary_path.exists():
            raise FirmwareBundleNotFoundError(
                f"ESP32 firmware bundle is incomplete. Missing file referenced by flash_args: {relative_name}"
            )

        flash_segments.extend([offset, str(binary_path)])

    return flash_options, flash_segments


def _run_command(command: list[str], log_callback: Callable[[str], None], stage_name: str) -> None:
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
        bufsize=1,
    )
    try:
        if process.stdout is not None:
            for line in process.stdout:
                cleaned = line.rstrip()
                if cleaned:
                    log_callback(cleaned)
        return_code = process.wait()
    finally:
        if process.stdout is not None:
            process.stdout.close()

    if return_code != 0:
        raise UploadCommandError(stage_name, return_code)


def _resolve_flasher_path() -> Path:
    if ESPTOOL_BUNDLED_PATH.exists():
        return ESPTOOL_BUNDLED_PATH

    esptool_on_path = shutil.which("esptool.exe") or shutil.which("esptool")
    if esptool_on_path:
        return Path(esptool_on_path)

    raise FlasherToolNotFoundError(
        "ESP32 flasher was not found. Reinstall the app or make esptool available on PATH."
    )


def _resolve_firmware_bundle_path() -> Path:
    if not TEST_FIRMWARE_DIR.exists():
        raise FirmwareBundleNotFoundError(
            f"ESP32 test firmware bundle was not found: {TEST_FIRMWARE_DIR}"
        )

    missing = [name for name in REQUIRED_FIRMWARE_FILES if not (TEST_FIRMWARE_DIR / name).exists()]
    if missing:
        raise FirmwareBundleNotFoundError(
            "ESP32 test firmware bundle is incomplete. Missing files: " + ", ".join(missing)
        )

    return TEST_FIRMWARE_DIR


def _extract_com_number(device: str) -> int | None:
    upper = device.upper()
    if not upper.startswith("COM"):
        return None
    suffix = upper[3:]
    if not suffix.isdigit():
        return None
    return int(suffix, 10)


def _looks_like_stlink(port: PortChoice) -> bool:
    joined = " ".join([port.device, port.description, port.manufacturer, port.hwid]).lower()
    return any(marker in joined for marker in STLINK_MARKERS)
