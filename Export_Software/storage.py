from __future__ import annotations

import csv
import json
from dataclasses import asdict, dataclass
from datetime import datetime
from pathlib import Path
from app_paths import get_data_dir
from serial_client import ExportRun
from translations import tr


BASE_DIR = get_data_dir()
EXPORTS_DIR = BASE_DIR / "exports"
LAST_OPERATION_FILE = BASE_DIR / "last_successful_operation.json"
SETTINGS_FILE = BASE_DIR / "settings.json"
CSV_HEADERS = ["Date and Time", "Test name", "Expected Result", "Actual result", "Pass/Fail"]


def _load_settings() -> dict:
    if not SETTINGS_FILE.exists():
        return {}
    try:
        return json.loads(SETTINGS_FILE.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {}


def _save_settings(settings: dict) -> None:
    BASE_DIR.mkdir(parents=True, exist_ok=True)
    SETTINGS_FILE.write_text(json.dumps(settings, indent=2), encoding="utf-8")


def get_exports_dir() -> Path:
    """Return the configured exports directory, falling back to the default."""
    settings = _load_settings()
    custom = settings.get("exports_dir")
    if custom:
        p = Path(custom)
        if p.is_absolute():
            return p
    return EXPORTS_DIR


def set_exports_dir(new_dir: Path) -> None:
    """Persist a new exports directory choice."""
    settings = _load_settings()
    settings["exports_dir"] = str(new_dir)
    _save_settings(settings)


def reset_exports_dir() -> None:
    """Remove the custom exports directory and revert to the default."""
    settings = _load_settings()
    settings.pop("exports_dir", None)
    _save_settings(settings)


def load_all_settings() -> dict:
    """Return the full settings dict (for the Settings dialog)."""
    return _load_settings()


def save_all_settings(settings: dict) -> None:
    """Write the full settings dict to disk."""
    _save_settings(settings)


DEFAULT_SETTINGS: dict = {
    "dark_mode": False,
    "language": "English",
    "confirm_before_extract": True,
    "auto_refresh_on_tab_switch": True,
    "reduced_motion": False,
}


@dataclass(frozen=True)
class ExportHistoryEntry:
    path: Path
    filename: str
    saved_time: str
    outcome: str
    mtime: float


@dataclass(frozen=True)
class LastOperationRecord:
    """Persisted summary of the last successful export or Rite-Hite Connect upload."""

    kind: str  # "export" | "upload"
    timestamp_iso: str
    device: str
    path: str | None
    detail: str

    def summarize(self) -> str:
        if self.kind == "export":
            path_part = self.path or ""
            name = Path(path_part).name if path_part else ""
            return tr("last_op_export", ts=self.timestamp_iso, name=name, device=self.device)
        return tr("last_op_upload", ts=self.timestamp_iso, device=self.device, detail=self.detail)


def ensure_exports_dir() -> Path:
    d = get_exports_dir()
    d.mkdir(parents=True, exist_ok=True)
    return d


def write_export_csv(export_run: ExportRun) -> Path:
    exports_dir = ensure_exports_dir()
    timestamp = datetime.now()
    timestamp_for_rows = timestamp.strftime("%Y-%m-%d %H:%M:%S")
    filename_stem = timestamp.strftime("%Y-%m-%d_%H-%M-%S")
    target = exports_dir / f"{filename_stem}_run{export_run.run_sequence}_{export_run.outcome}.csv"
    target = _deduplicate_path(target)

    with target.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle, quoting=csv.QUOTE_MINIMAL)
        writer.writerow(CSV_HEADERS)
        for row in export_run.rows:
            writer.writerow(
                [
                    timestamp_for_rows,
                    row.test_name,
                    row.expected_result,
                    row.actual_result,
                    row.pass_fail,
                ]
            )

    return target


def list_export_history() -> list[ExportHistoryEntry]:
    exports_dir = ensure_exports_dir()
    entries: list[ExportHistoryEntry] = []

    for path in exports_dir.glob("*.csv"):
        stat = path.stat()
        entries.append(
            ExportHistoryEntry(
                path=path,
                filename=path.name,
                saved_time=datetime.fromtimestamp(stat.st_mtime).strftime("%Y-%m-%d %H:%M:%S"),
                outcome=_parse_outcome(path.name),
                mtime=stat.st_mtime,
            )
        )

    entries.sort(key=lambda entry: entry.path.stat().st_mtime, reverse=True)
    return entries


def _deduplicate_path(target: Path) -> Path:
    if not target.exists():
        return target

    counter = 2
    while True:
        candidate = target.with_name(f"{target.stem}_{counter}{target.suffix}")
        if not candidate.exists():
            return candidate
        counter += 1


def _parse_outcome(filename: str) -> str:
    upper_name = filename.upper()
    if "_PASS" in upper_name:
        return "PASS"
    if "_FAIL" in upper_name:
        return "FAIL"
    return "UNKNOWN"


def load_last_operation() -> LastOperationRecord | None:
    if not LAST_OPERATION_FILE.exists():
        return None
    try:
        raw = json.loads(LAST_OPERATION_FILE.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError, TypeError):
        return None
    if not isinstance(raw, dict):
        return None
    try:
        return LastOperationRecord(
            kind=str(raw["kind"]),
            timestamp_iso=str(raw["timestamp_iso"]),
            device=str(raw["device"]),
            path=str(raw["path"]) if raw.get("path") else None,
            detail=str(raw.get("detail", "")),
        )
    except KeyError:
        return None


def save_last_successful_export(saved_path: Path, port_device: str) -> None:
    record = LastOperationRecord(
        kind="export",
        timestamp_iso=datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        device=port_device,
        path=str(saved_path),
        detail=saved_path.name,
    )
    _write_last_operation(record)


def save_last_successful_upload(upload_port: str, detail: str) -> None:
    record = LastOperationRecord(
        kind="upload",
        timestamp_iso=datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        device=upload_port,
        path=None,
        detail=detail,
    )
    _write_last_operation(record)


def _write_last_operation(record: LastOperationRecord) -> None:
    BASE_DIR.mkdir(parents=True, exist_ok=True)
    payload = asdict(record)
    LAST_OPERATION_FILE.write_text(json.dumps(payload, indent=2), encoding="utf-8")
