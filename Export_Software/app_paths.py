from __future__ import annotations

import os
import sys
from pathlib import Path


COMPANY_DIR = "Rite-Hite"
APP_DIR = "EOL_Export_Software"
LEGACY_APP_DIR = "EOL Export Software"


def is_frozen() -> bool:
    return bool(getattr(sys, "frozen", False))


def get_bundle_dir() -> Path:
    if is_frozen():
        return Path(getattr(sys, "_MEIPASS"))
    return Path(__file__).resolve().parent


def get_app_dir() -> Path:
    if is_frozen():
        return Path(sys.executable).resolve().parent
    return Path(__file__).resolve().parent


def get_data_dir() -> Path:
    if not is_frozen():
        return get_app_dir()

    local_appdata = os.environ.get("LOCALAPPDATA")
    if local_appdata:
        company_dir = Path(local_appdata) / COMPANY_DIR
        data_dir = company_dir / APP_DIR
        legacy_data_dir = company_dir / LEGACY_APP_DIR
        if legacy_data_dir.exists() and not data_dir.exists():
            try:
                company_dir.mkdir(parents=True, exist_ok=True)
                legacy_data_dir.rename(data_dir)
            except OSError:
                pass
        return data_dir

    return get_app_dir()
