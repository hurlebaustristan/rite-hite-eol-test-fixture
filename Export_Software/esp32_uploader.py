from __future__ import annotations

import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Callable

from app_paths import get_bundle_dir
from serial_client import PortChoice, list_available_ports
from translations import tr


ARDUINO_CLI_BUNDLED_PATH = Path(
    r"C:\Users\hurlebaust\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
)
SOURCE_TEST_SKETCH_DIR = Path(r"C:\TouchGFXProjects\EOL_TestFixture_Final\ArduinoIDE\EOL_RiteHite_Final")
TEST_SKETCH_NAME = "EOL_RiteHite_Final"
TEST_BOARD_NAME = "ESP32S3 Dev Module"
TEST_FQBN = "esp32:esp32:esp32s3"
TEST_FIRMWARE_DIR = get_bundle_dir() / "esp32_test_firmware"
TEST_BUNDLED_SKETCH_DIR = get_bundle_dir() / "esp32_test_sketch"
REQUIRED_FIRMWARE_FILES = (
    "EOL_RiteHite_Final.ino.bin",
    "EOL_RiteHite_Final.ino.bootloader.bin",
    "EOL_RiteHite_Final.ino.partitions.bin",
    "partitions.csv",
    "build.options.json",
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
    cli_path: Path
    sketch_dir: Path
    sketch_name: str
    fqbn: str
    board_name: str
    build_path: Path


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


class ArduinoCliNotFoundError(Esp32UploadError):
    pass


class SketchPathNotFoundError(Esp32UploadError):
    pass


class FirmwareBundleNotFoundError(Esp32UploadError):
    pass


class UploadCommandError(Esp32UploadError):
    def __init__(self, stage_name: str, return_code: int) -> None:
        super().__init__(f"{stage_name} failed. Arduino CLI exited with code {return_code}.")
        self.stage_name = stage_name
        self.return_code = return_code


def get_default_upload_config() -> Esp32UploadConfig:
    cli_path = _resolve_arduino_cli_path()
    sketch_dir = _resolve_sketch_dir()

    return Esp32UploadConfig(
        cli_path=cli_path,
        sketch_dir=sketch_dir,
        sketch_name=TEST_SKETCH_NAME,
        fqbn=TEST_FQBN,
        board_name=TEST_BOARD_NAME,
        build_path=_resolve_firmware_bundle_path(),
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
    """Summarize programmer detection for the connection readiness UI."""
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
    upload_command = [
        str(config.cli_path),
        "upload",
        "-p",
        selection.upload_port,
        "--fqbn",
        config.fqbn,
        "--build-path",
        str(config.build_path),
        "--no-color",
        str(config.sketch_dir),
    ]

    log_callback(f"Using Arduino CLI: {config.cli_path}")
    log_callback(f"Using prebuilt firmware bundle: {config.build_path}")
    log_callback(f"Uploading {config.sketch_name} to {selection.upload_port}...")
    _run_command(upload_command, log_callback, "Upload")
    log_callback(f"Upload succeeded on {selection.upload_port}.")


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


def _resolve_arduino_cli_path() -> Path:
    if ARDUINO_CLI_BUNDLED_PATH.exists():
        return ARDUINO_CLI_BUNDLED_PATH

    cli_on_path = shutil.which("arduino-cli")
    if cli_on_path:
        return Path(cli_on_path)

    raise ArduinoCliNotFoundError(
        "Arduino CLI was not found. Install Arduino IDE or make arduino-cli available on PATH."
    )


def _resolve_sketch_dir() -> Path:
    if TEST_BUNDLED_SKETCH_DIR.exists():
        return TEST_BUNDLED_SKETCH_DIR
    if SOURCE_TEST_SKETCH_DIR.exists():
        return SOURCE_TEST_SKETCH_DIR
    raise SketchPathNotFoundError(
        f"ESP32 test sketch folder was not found: {SOURCE_TEST_SKETCH_DIR}"
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
