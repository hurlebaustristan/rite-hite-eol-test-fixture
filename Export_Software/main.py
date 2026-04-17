from __future__ import annotations

import os
import subprocess
import sys
from datetime import datetime
from pathlib import Path

from app_paths import get_bundle_dir
from PySide6.QtCore import QEvent, QObject, QThread, QTimer, Qt, QUrl, Signal
from PySide6.QtGui import QAction, QBrush, QColor, QDesktopServices, QFont, QGuiApplication, QIcon, QKeySequence, QPixmap, QShortcut, QTextCursor
from PySide6.QtWidgets import (
    QApplication,
    QCheckBox,
    QComboBox,
    QDialog,
    QDialogButtonBox,
    QFileDialog,
    QFormLayout,
    QFrame,
    QGraphicsDropShadowEffect,
    QGroupBox,
    QHBoxLayout,
    QHeaderView,
    QLabel,
    QLineEdit,
    QMainWindow,
    QMenu,
    QMessageBox,
    QPlainTextEdit,
    QProgressBar,
    QPushButton,
    QScrollArea,
    QSizePolicy,
    QStackedWidget,
    QStyle,
    QTableWidget,
    QTableWidgetItem,
    QTextEdit,
    QToolButton,
    QVBoxLayout,
    QWidget,
)

from theme import (
    TOKENS,
    apply_shadow,
    build_dark_stylesheet,
    build_stylesheet,
    color as token_color,
    qta_icon,
    set_status_pill,
    tune_shadow,
)
from widgets import LogDrawer, Sidebar, SidebarItem, StepProgressBar, StepState

from esp32_uploader import (
    Esp32UploadChoice,
    Esp32UploadConfig,
    Esp32UploadError,
    describe_esp_prog_detection,
    discover_upload_targets,
    get_default_upload_config,
    run_upload,
)
from serial_client import (
    AutoTestAlreadyRunningError,
    AutoTestStartError,
    DeviceAlreadyExportedError,
    DeviceEmptyError,
    DeviceRunningError,
    ExportProtocolClient,
    NoPortsAvailableError,
    PortChoice,
    PortSelectionRequiredError,
    detect_preferred_port,
    list_available_ports,
    looks_like_stlink_port,
)
from translations import get_language, set_language, tr

from storage import (
    DEFAULT_SETTINGS,
    EXPORTS_DIR,
    ExportHistoryEntry,
    ensure_exports_dir,
    get_exports_dir,
    list_export_history,
    load_all_settings,
    load_last_operation,
    reset_exports_dir,
    save_all_settings,
    save_last_successful_export,
    save_last_successful_upload,
    set_exports_dir,
    write_export_csv,
)

OCT_STEP_INDEX_UPLOAD = 0
OCT_STEP_INDEX_COMMS = 1
OCT_STEP_INDEX_DIGITAL_INPUTS = 2
OCT_STEP_INDEX_ANALOG_INPUTS = 3
OCT_STEP_INDEX_DIGITAL_OUTPUTS = 4
OCT_STEP_INDEX_RELAY_OUTPUTS = 5
OCT_STEP_INDEX_BUTTONS = 6
OCT_STEP_INDEX_LEDS = 7
OCT_STEP_INDEX_EXTRACT = 8

OCT_PHASE_TO_STEP_KEY = {
    "COMMS": "oct_step_comms",
    "DIGITAL_INPUTS": "oct_step_digital_inputs",
    "ANALOG_INPUTS": "oct_step_analog_inputs",
    "DIGITAL_OUTPUTS": "oct_step_digital_outputs",
    "RELAY_OUTPUTS": "oct_step_relay_outputs",
    "BUTTONS": "oct_step_buttons",
    "LEDS": "oct_step_leds",
}

OCT_PHASE_TO_STEP_INDEX = {
    "COMMS": OCT_STEP_INDEX_COMMS,
    "DIGITAL_INPUTS": OCT_STEP_INDEX_DIGITAL_INPUTS,
    "ANALOG_INPUTS": OCT_STEP_INDEX_ANALOG_INPUTS,
    "DIGITAL_OUTPUTS": OCT_STEP_INDEX_DIGITAL_OUTPUTS,
    "RELAY_OUTPUTS": OCT_STEP_INDEX_RELAY_OUTPUTS,
    "BUTTONS": OCT_STEP_INDEX_BUTTONS,
    "LEDS": OCT_STEP_INDEX_LEDS,
}


def one_click_phase_label(phase: str | None) -> str:
    if not phase:
        return ""
    key = OCT_PHASE_TO_STEP_KEY.get(phase)
    if key is not None:
        return tr(key)
    return phase.replace("_", " ").title()


def one_click_phase_step_index(phase: str | None) -> int | None:
    if not phase:
        return None
    return OCT_PHASE_TO_STEP_INDEX.get(phase)


class PortPickerDialog(QDialog):
    def __init__(self, ports: list[PortChoice], reason: str, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setWindowTitle("Select Serial Port")
        self.setModal(True)
        self._ports = ports

        layout = QVBoxLayout(self)

        reason_label = QLabel(reason)
        reason_label.setWordWrap(True)
        layout.addWidget(reason_label)

        self.port_combo = QComboBox()
        for port in ports:
            self.port_combo.addItem(port.display_name, port)
        layout.addWidget(self.port_combo)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

    def selected_port(self) -> PortChoice | None:
        current = self.port_combo.currentData()
        if isinstance(current, PortChoice):
            return current
        return None


class UploadTargetPickerDialog(QDialog):
    def __init__(self, choices: list[Esp32UploadChoice], reason: str, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setWindowTitle("Select Rite-Hite Connect Upload Port")
        self.setModal(True)
        self._choices = choices

        layout = QVBoxLayout(self)

        reason_label = QLabel(reason)
        reason_label.setWordWrap(True)
        layout.addWidget(reason_label)

        self.choice_combo = QComboBox()
        for choice in choices:
            self.choice_combo.addItem(choice.label, choice)
        layout.addWidget(self.choice_combo)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

    def selected_choice(self) -> Esp32UploadChoice | None:
        current = self.choice_combo.currentData()
        if isinstance(current, Esp32UploadChoice):
            return current
        return None


class UploadConfirmDialog(QDialog):
    def __init__(self, config: Esp32UploadConfig, choice: Esp32UploadChoice, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setWindowTitle("Confirm Rite-Hite Connect Test Firmware Upload")
        self.setModal(True)
        self.setMinimumWidth(520)

        layout = QVBoxLayout(self)
        layout.setSpacing(14)

        title = QLabel("Ready to upload Rite-Hite Connect test firmware")
        title.setObjectName("sectionTitle")
        layout.addWidget(title)

        intro = QLabel(
            "Review the detected board and upload port below. Click Upload to send the test firmware to the Rite-Hite Connect module."
        )
        intro.setWordWrap(True)
        layout.addWidget(intro)

        details_box = QFrame()
        details_box.setObjectName("uploadConfirmBox")
        details_layout = QVBoxLayout(details_box)
        details_layout.setContentsMargins(14, 12, 14, 12)
        details_layout.setSpacing(8)

        details = [
            f"Board: {config.board_name}",
            f"Sketch: {config.sketch_name}",
            f"Upload port: {choice.upload_port}",
            f"Detected pair: {' / '.join(choice.pair_ports)}" if choice.is_pair else f"Manual port: {choice.upload_port}",
            "Rule: the upload port is the lower COM port in the programmer pair.",
        ]
        for line in details:
            label = QLabel(line)
            label.setWordWrap(True)
            details_layout.addWidget(label)

        layout.addWidget(details_box)

        note = QLabel("If this matches your programmer connection, click Upload. Otherwise click Cancel.")
        note.setWordWrap(True)
        layout.addWidget(note)

        buttons = QDialogButtonBox()
        upload_button = buttons.addButton("Upload", QDialogButtonBox.AcceptRole)
        cancel_button = buttons.addButton("Cancel", QDialogButtonBox.RejectRole)
        upload_button.setObjectName("primaryExportButton")
        cancel_button.setObjectName("utilityButton")
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)


class UploadErrorDialog(QDialog):
    def __init__(self, message: str, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setWindowTitle("Rite-Hite Connect Upload Failed")
        self.setModal(True)
        self.setMinimumWidth(540)

        layout = QVBoxLayout(self)
        layout.setSpacing(14)

        top_row = QHBoxLayout()
        top_row.setSpacing(12)
        layout.addLayout(top_row)

        icon_label = QLabel("X")
        icon_label.setObjectName("uploadErrorIcon")
        icon_label.setAlignment(Qt.AlignCenter)
        icon_label.setFixedSize(44, 44)
        top_row.addWidget(icon_label, 0, Qt.AlignTop)

        text_column = QVBoxLayout()
        text_column.setSpacing(8)
        top_row.addLayout(text_column, 1)

        title = QLabel("The Rite-Hite Connect upload could not be completed.")
        title.setObjectName("sectionTitle")
        text_column.addWidget(title)

        intro = QLabel("Review the message below, correct the issue, and then try the upload again.")
        intro.setWordWrap(True)
        text_column.addWidget(intro)

        details_box = QFrame()
        details_box.setObjectName("uploadErrorBox")
        details_layout = QVBoxLayout(details_box)
        details_layout.setContentsMargins(14, 12, 14, 12)
        details_layout.setSpacing(0)

        details = QLabel(message)
        details.setObjectName("uploadErrorMessage")
        details.setWordWrap(True)
        details_layout.addWidget(details)
        layout.addWidget(details_box)

        buttons = QDialogButtonBox()
        close_button = buttons.addButton("Close", QDialogButtonBox.AcceptRole)
        close_button.setObjectName("utilityButton")
        buttons.accepted.connect(self.accept)
        layout.addWidget(buttons)


class CsvPreviewDialog(QDialog):
    def __init__(self, path: Path, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setWindowTitle(f"Preview — {path.name}")
        self.setMinimumSize(560, 420)

        layout = QVBoxLayout(self)
        preview = QPlainTextEdit()
        preview.setReadOnly(True)
        preview.setPlaceholderText("Could not read this file.")
        try:
            raw = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            raw = ""
        lines = raw.splitlines()[:50]
        preview.setPlainText("\n".join(lines) if lines else "(empty file)")
        layout.addWidget(preview)

        button_row = QHBoxLayout()
        button_row.addStretch(1)
        close_btn = QPushButton("Close")
        close_btn.setObjectName("utilityButton")
        close_btn.clicked.connect(self.accept)
        button_row.addWidget(close_btn)
        layout.addLayout(button_row)


class SettingsDialog(QDialog):
    """Application settings dialog."""

    settings_changed = Signal(dict)

    def __init__(self, current_settings: dict, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setWindowTitle(tr("settings"))
        self.setFixedWidth(420)
        self.setModal(True)
        self._settings = dict(current_settings)

        self.setStyleSheet(self._dialog_stylesheet())

        layout = QVBoxLayout(self)
        layout.setSpacing(0)
        layout.setContentsMargins(0, 0, 0, 0)

        header = QWidget()
        header.setObjectName("settingsHeader")
        header_layout = QHBoxLayout(header)
        header_layout.setContentsMargins(20, 14, 20, 10)
        gear_icon = QLabel("\u2699")
        gear_icon.setStyleSheet("font-size: 18px; color: #e0194a; background: transparent;")
        header_layout.addWidget(gear_icon)
        header_layout.addSpacing(6)
        title = QLabel(tr("settings"))
        title.setStyleSheet("font-size: 17px; font-weight: 800; color: #1a1a1a; background: transparent;")
        header_layout.addWidget(title)
        header_layout.addStretch(1)
        layout.addWidget(header)

        sep = QFrame()
        sep.setFixedHeight(1)
        sep.setStyleSheet("background: #e0e0e0;")
        layout.addWidget(sep)

        body = QWidget()
        body_layout = QVBoxLayout(body)
        body_layout.setSpacing(12)
        body_layout.setContentsMargins(20, 14, 20, 14)

        body_layout.addWidget(self._build_section_label(tr("settings_appearance")))

        self.dark_mode_check = QCheckBox(tr("settings_dark_mode"))
        self.dark_mode_check.setChecked(self._settings.get("dark_mode", False))
        body_layout.addWidget(self.dark_mode_check)

        font_row = QHBoxLayout()
        font_row.setSpacing(10)
        font_label = QLabel(tr("settings_font_size"))
        font_label.setStyleSheet("font-size: 13px; font-weight: 600; color: #1a1a1a;")
        font_row.addWidget(font_label)
        self.font_size_combo = QComboBox()
        self.font_size_combo.addItems(["Small", "Medium", "Large"])
        self.font_size_combo.setCurrentText(self._settings.get("font_size", "Medium"))
        self.font_size_combo.setFixedWidth(120)
        font_row.addWidget(self.font_size_combo)
        font_row.addStretch(1)
        body_layout.addLayout(font_row)

        body_layout.addSpacing(4)
        body_layout.addWidget(self._build_section_label(tr("settings_language_section")))

        lang_row = QHBoxLayout()
        lang_row.setSpacing(10)
        lang_label = QLabel(tr("settings_display_language"))
        lang_label.setStyleSheet("font-size: 13px; font-weight: 600; color: #1a1a1a;")
        lang_row.addWidget(lang_label)
        self.language_combo = QComboBox()
        self.language_combo.addItems(["English", "Spanish", "Hmong"])
        self.language_combo.setCurrentText(self._settings.get("language", "English"))
        self.language_combo.setFixedWidth(140)
        lang_row.addWidget(self.language_combo)
        lang_row.addStretch(1)
        body_layout.addLayout(lang_row)
        lang_note = QLabel(tr("settings_language_note"))
        lang_note.setStyleSheet("color: #999999; font-size: 11px; font-style: italic;")
        body_layout.addWidget(lang_note)

        body_layout.addSpacing(4)
        body_layout.addWidget(self._build_section_label(tr("settings_behavior")))

        self.confirm_extract_check = QCheckBox(tr("settings_confirm_extract"))
        self.confirm_extract_check.setChecked(self._settings.get("confirm_before_extract", True))
        body_layout.addWidget(self.confirm_extract_check)

        self.auto_refresh_check = QCheckBox(tr("settings_auto_refresh"))
        self.auto_refresh_check.setChecked(self._settings.get("auto_refresh_on_tab_switch", True))
        body_layout.addWidget(self.auto_refresh_check)

        self.reduced_motion_check = QCheckBox(tr("settings_reduced_motion"))
        self.reduced_motion_check.setChecked(self._settings.get("reduced_motion", False))
        body_layout.addWidget(self.reduced_motion_check)

        layout.addWidget(body, 1)

        footer_sep = QFrame()
        footer_sep.setFixedHeight(1)
        footer_sep.setStyleSheet("background: #e0e0e0;")
        layout.addWidget(footer_sep)

        footer = QWidget()
        footer.setObjectName("settingsFooter")
        footer_layout = QHBoxLayout(footer)
        footer_layout.setContentsMargins(20, 12, 20, 12)
        footer_layout.addStretch(1)

        cancel_btn = QPushButton(tr("cancel"))
        cancel_btn.setObjectName("settingsCancelButton")
        cancel_btn.clicked.connect(self.reject)
        footer_layout.addWidget(cancel_btn)

        footer_layout.addSpacing(8)

        save_btn = QPushButton(tr("save"))
        save_btn.setObjectName("settingsSaveButton")
        save_btn.clicked.connect(self._on_save)
        footer_layout.addWidget(save_btn)

        layout.addWidget(footer)

    @staticmethod
    def _build_section_label(text: str) -> QLabel:
        label = QLabel(text.upper())
        label.setStyleSheet(
            "font-size: 11px; font-weight: 700; color: #e0194a; "
            "letter-spacing: 1.2px; background: transparent; padding: 2px 0;"
        )
        return label

    @staticmethod
    def _dialog_stylesheet() -> str:
        return """
            QDialog {
                background: #ffffff;
            }
            QWidget#settingsHeader {
                background: #fafafa;
            }
            QWidget#settingsFooter {
                background: #fafafa;
            }
            QCheckBox {
                font-size: 13px;
                font-weight: 600;
                color: #1a1a1a;
                spacing: 8px;
            }
            QCheckBox::indicator {
                width: 18px;
                height: 18px;
                border: 2px solid #cccccc;
                border-radius: 4px;
                background: #ffffff;
            }
            QCheckBox::indicator:hover {
                border-color: #e0194a;
            }
            QCheckBox::indicator:checked {
                background: #e0194a;
                border-color: #e0194a;
            }
            QComboBox {
                background: #ffffff;
                color: #1a1a1a;
                border: 1px solid #e0e0e0;
                border-radius: 6px;
                padding: 6px 10px;
                min-height: 30px;
                font-size: 13px;
            }
            QComboBox QAbstractItemView {
                background: #ffffff;
                color: #1a1a1a;
                selection-background-color: #fce4ea;
                selection-color: #1a1a1a;
                border: 1px solid #e0e0e0;
                border-radius: 6px;
            }
            QComboBox::drop-down {
                border: none;
                width: 28px;
            }
            QPushButton#settingsSaveButton {
                background: #e0194a;
                color: #ffffff;
                border: none;
                border-radius: 6px;
                font-size: 13px;
                font-weight: 700;
                padding: 8px 22px;
                min-height: 34px;
            }
            QPushButton#settingsSaveButton:hover {
                background: #c91542;
            }
            QPushButton#settingsSaveButton:pressed {
                background: #af1139;
            }
            QPushButton#settingsCancelButton {
                background: transparent;
                color: #666666;
                border: 1px solid #e0e0e0;
                border-radius: 6px;
                font-size: 13px;
                font-weight: 600;
                padding: 8px 18px;
                min-height: 34px;
            }
            QPushButton#settingsCancelButton:hover {
                background: #f5f5f5;
                color: #1a1a1a;
                border-color: #cccccc;
            }
        """

    def _on_save(self) -> None:
        self._settings["dark_mode"] = self.dark_mode_check.isChecked()
        self._settings["font_size"] = self.font_size_combo.currentText()
        self._settings["language"] = self.language_combo.currentText()
        self._settings["confirm_before_extract"] = self.confirm_extract_check.isChecked()
        self._settings["auto_refresh_on_tab_switch"] = self.auto_refresh_check.isChecked()
        self._settings["reduced_motion"] = self.reduced_motion_check.isChecked()
        self.accept()

    def get_settings(self) -> dict:
        return dict(self._settings)


class Esp32UploadWorker(QObject):
    log_message = Signal(str)
    upload_succeeded = Signal(str)
    upload_failed = Signal(str)
    finished = Signal()

    def __init__(self, config: Esp32UploadConfig, choice: Esp32UploadChoice) -> None:
        super().__init__()
        self._config = config
        self._choice = choice

    def run(self) -> None:
        try:
            run_upload(self._config, self._choice, self.log_message.emit)
            self.upload_succeeded.emit(
                f"Rite-Hite Connect test firmware upload completed successfully on {self._choice.upload_port}."
            )
        except Exception as exc:
            self.upload_failed.emit(str(exc))
        finally:
            self.finished.emit()


class OneClickWorker(QObject):
    """Runs the full one-click test sequence on a background thread."""

    POLL_INTERVAL_SEC = 0.5
    POLL_TIMEOUT_SEC = 300.0
    STEP_ADVANCE_DELAY_SEC = 0.18

    stage_changed = Signal(int, str)
    log_message = Signal(str)
    succeeded = Signal(str)
    failed = Signal(str)
    finished = Signal()

    def __init__(
        self,
        upload_config: Esp32UploadConfig,
        upload_choice: Esp32UploadChoice,
        fixture_port: str,
    ) -> None:
        super().__init__()
        self._upload_config = upload_config
        self._upload_choice = upload_choice
        self._fixture_port = fixture_port
        self._client = ExportProtocolClient()

    ESP32_BOOT_DELAY_SEC = 5.0

    def _advance_stepper(self, current_step: int | None, target_step: int, pause_fn) -> int:
        if current_step is None or target_step <= current_step:
            self.stage_changed.emit(target_step, "")
            return target_step

        for step_index in range(current_step + 1, target_step + 1):
            self.stage_changed.emit(step_index, "")
            if step_index != target_step:
                pause_fn(self.STEP_ADVANCE_DELAY_SEC)
        return target_step

    def run(self) -> None:
        import time

        try:
            active_step_index = OCT_STEP_INDEX_UPLOAD
            current_phase = None

            self.stage_changed.emit(OCT_STEP_INDEX_UPLOAD, tr("oct_stage_uploading"))
            self.log_message.emit(tr("oct_stage_uploading"))
            run_upload(self._upload_config, self._upload_choice, self.log_message.emit)
            self.log_message.emit(tr("oct_upload_done"))

            self.log_message.emit(tr("oct_esp_boot_wait"))
            time.sleep(self.ESP32_BOOT_DELAY_SEC)

            self.log_message.emit(tr("oct_stage_starting"))
            self._client.start_auto_test(self._fixture_port)
            self.log_message.emit(tr("oct_auto_started"))
            active_step_index = self._advance_stepper(active_step_index, OCT_STEP_INDEX_COMMS, time.sleep)
            current_phase = "COMMS"
            self.log_message.emit(f"Fixture phase: {one_click_phase_label(current_phase)}")

            self.log_message.emit(tr("oct_stage_waiting"))
            deadline = time.monotonic() + self.POLL_TIMEOUT_SEC
            status = None
            visual_prompt_logged = False
            while time.monotonic() < deadline:
                time.sleep(self.POLL_INTERVAL_SEC)
                status = self._client.get_status(self._fixture_port)

                status_step_index = one_click_phase_step_index(status.phase)
                if status.state == "WAITING_VISUAL" and status_step_index is None:
                    status_step_index = OCT_STEP_INDEX_LEDS

                if status.phase and status.phase != current_phase:
                    active_step_index = self._advance_stepper(active_step_index, status_step_index or active_step_index, time.sleep)
                    current_phase = status.phase
                    self.log_message.emit(f"Fixture phase: {one_click_phase_label(status.phase)}")
                elif status_step_index is not None and status_step_index > active_step_index:
                    active_step_index = self._advance_stepper(active_step_index, status_step_index, time.sleep)

                if status.state == "WAITING_VISUAL":
                    if not visual_prompt_logged:
                        self.log_message.emit(tr("oct_visual_prompt"))
                        visual_prompt_logged = True
                else:
                    visual_prompt_logged = False

                if status.state in ("READY", "EXPORTED"):
                    break
                if status.state not in ("RUNNING", "EMPTY", "WAITING_VISUAL"):
                    self.failed.emit(tr("oct_unexpected_state", state=status.state))
                    return
            else:
                self.failed.emit(tr("oct_timeout"))
                return

            if status is None or status.state not in ("READY", "EXPORTED"):
                self.failed.emit(tr("oct_unexpected_state", state=status.state if status else "None"))
                return

            active_step_index = self._advance_stepper(active_step_index, OCT_STEP_INDEX_EXTRACT, time.sleep)
            self.log_message.emit(tr("oct_stage_extracting"))
            export_run = self._client.export_latest_run(self._fixture_port)
            saved_path = write_export_csv(export_run)
            save_last_successful_export(saved_path, self._fixture_port)
            self.log_message.emit(tr("export_saved", seq=export_run.run_sequence, outcome=export_run.outcome, name=saved_path.name))

            self.succeeded.emit(tr("oct_complete", outcome=export_run.outcome, name=saved_path.name))
        except Exception as exc:
            self.failed.emit(str(exc))
        finally:
            self.finished.emit()


PAGE_INDEX_FILE_EXPLORER = 0
PAGE_INDEX_DEVICE_TOOLS = 1
PAGE_INDEX_ONE_CLICK = 2


class ExportAppWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("EOL Export Software")
        self.resize(1240, 760)
        self.client = ExportProtocolClient()
        self.assets_dir = get_bundle_dir() / "assets"
        self.logo_path = self.assets_dir / "rite_hite_logo.svg"
        self.icon_path = self._resolve_app_icon_path()
        self.upload_thread: QThread | None = None
        self.upload_worker: Esp32UploadWorker | None = None
        self._last_upload_choice: Esp32UploadChoice | None = None
        self.oneclick_thread: QThread | None = None
        self.oneclick_worker: OneClickWorker | None = None
        self._all_history_entries: list[ExportHistoryEntry] = []
        self._app_settings: dict = {**DEFAULT_SETTINGS, **load_all_settings()}
        set_language(self._app_settings.get("language", "English"))

        if self.icon_path is not None:
            self.setWindowIcon(QIcon(str(self.icon_path)))

        ensure_exports_dir()

        central = QWidget()
        central.setObjectName("appRoot")
        self.setCentralWidget(central)

        root_layout = QVBoxLayout(central)
        root_layout.setContentsMargins(20, 16, 20, 16)
        root_layout.setSpacing(14)

        root_layout.addWidget(self._build_masthead())

        self.flash_banner = QLabel()
        self.flash_banner.setObjectName("flashBanner")
        self.flash_banner.setAlignment(Qt.AlignCenter)
        self.flash_banner.setWordWrap(True)
        self.flash_banner.setVisible(False)
        self.flash_banner.setFixedHeight(44)
        root_layout.addWidget(self.flash_banner)

        body_layout = QHBoxLayout()
        body_layout.setSpacing(14)

        reduced = bool(self._app_settings.get("reduced_motion", False))

        nav_items = [
            SidebarItem(
                key="file_explorer",
                label=tr("tab_file_explorer"),
                icon_name="fa6s.folder-open",
                tooltip=tr("tab_file_explorer"),
            ),
            SidebarItem(
                key="device_tools",
                label=tr("tab_device_tools"),
                icon_name="fa6s.screwdriver-wrench",
                tooltip=tr("tab_device_tools"),
            ),
            SidebarItem(
                key="one_click_test",
                label=tr("tab_one_click_test"),
                icon_name="fa6s.circle-play",
                tooltip=tr("tab_one_click_test"),
            ),
        ]
        self.sidebar = Sidebar(
            nav_items,
            settings_label=tr("nav_settings"),
            collapse_label=tr("nav_collapse"),
            expand_label=tr("nav_expand"),
            reduced_motion=reduced,
            parent=self,
        )
        self.sidebar.pageChanged.connect(self._on_main_page_changed)
        self.sidebar.settingsRequested.connect(self._open_settings)
        apply_shadow(self.sidebar, "sm")
        body_layout.addWidget(self.sidebar, 0)

        content_column = QVBoxLayout()
        content_column.setSpacing(12)

        self.pages = QStackedWidget()
        self.pages.setObjectName("pagesStack")
        self.pages.addWidget(self._wrap_page(self._build_file_explorer_tab()))
        self.pages.addWidget(self._wrap_page(self._build_export_tab()))
        self.pages.addWidget(self._wrap_page(self._build_one_click_tab()))
        apply_shadow(self.pages, "md")
        content_column.addWidget(self.pages, 1)

        self.progress_bar = QProgressBar()
        self.progress_bar.setObjectName("operationProgress")
        self.progress_bar.setRange(0, 0)
        self.progress_bar.setTextVisible(False)
        self.progress_bar.setFixedHeight(4)
        self.progress_bar.setVisible(False)
        content_column.addWidget(self.progress_bar)

        self.log_drawer = LogDrawer(
            title=tr("activity_log"),
            copy_label=tr("copy"),
            clear_label=tr("clear"),
            reduced_motion=reduced,
            parent=self,
        )
        self.log_drawer.expandedChanged.connect(self._schedule_layout_refresh)
        apply_shadow(self.log_drawer, "sm")
        content_column.addWidget(self.log_drawer, 0)

        body_layout.addLayout(content_column, 1)
        root_layout.addLayout(body_layout, 1)

        self._apply_current_settings()
        self._retranslate_ui()
        self.set_status_message(tr("ready_message"))

        self.connection_timer = QTimer(self)
        self.connection_timer.setInterval(4000)
        self.connection_timer.timeout.connect(self._on_connection_timer)
        self._on_main_page_changed(self.sidebar.active_index())
        self._update_last_operation_ui()

        QShortcut(QKeySequence("F5"), self).activated.connect(self.refresh_history)
        QShortcut(QKeySequence("Ctrl+E"), self).activated.connect(self.handle_extract_clicked)
        QShortcut(QKeySequence("Ctrl+U"), self).activated.connect(self.handle_upload_test_clicked)
        QShortcut(QKeySequence("Ctrl+O"), self).activated.connect(self.open_exports_folder)
        QShortcut(QKeySequence("Ctrl+L"), self).activated.connect(self.log_drawer.toggle)
        QShortcut(QKeySequence("Ctrl+B"), self).activated.connect(self.sidebar.toggle_collapsed)
        QShortcut(QKeySequence(Qt.Key.Key_Escape), self).activated.connect(
            self._handle_escape_window_mode
        )

    def _resolve_app_icon_path(self) -> Path | None:
        candidates = (
            self.assets_dir / "rite_hite_app_icon.ico",
            self.assets_dir / "rite_hite_app_icon.png",
            self.assets_dir / "rite_hite_logo.svg",
        )
        for candidate in candidates:
            if candidate.exists():
                return candidate
        return None

    def _wrap_page(self, page: QWidget) -> QScrollArea:
        shell = QWidget()
        shell.setObjectName("pageRoot")
        shell_layout = QVBoxLayout(shell)
        shell_layout.setContentsMargins(0, 0, 0, 0)
        shell_layout.setSpacing(0)
        shell_layout.addWidget(page)

        scroll = QScrollArea()
        scroll.setObjectName("pageScrollArea")
        scroll.setFrameShape(QFrame.Shape.NoFrame)
        scroll.setWidgetResizable(True)
        scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        scroll.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        scroll.setWidget(shell)
        return scroll

    def _build_masthead(self) -> QWidget:
        masthead = QWidget()
        masthead.setObjectName("masthead")
        apply_shadow(masthead, "sm")

        outer = QVBoxLayout(masthead)
        outer.setContentsMargins(0, 0, 0, 0)
        outer.setSpacing(0)

        content = QWidget()
        content.setObjectName("mastheadContent")
        content_layout = QHBoxLayout(content)
        content_layout.setContentsMargins(22, 18, 22, 16)
        content_layout.setSpacing(20)

        png_path = self.assets_dir / "rite-hite-logo-600.png"
        logo_label = QLabel()
        logo_label.setObjectName("brandLogo")
        if png_path.exists():
            pixmap = QPixmap(str(png_path))
            scaled = pixmap.scaledToHeight(56, Qt.TransformationMode.SmoothTransformation)
            logo_label.setPixmap(scaled)
            logo_label.setFixedSize(scaled.width(), scaled.height())
        elif self.logo_path.exists():
            pixmap = QPixmap(str(self.logo_path))
            if not pixmap.isNull():
                scaled = pixmap.scaledToHeight(56, Qt.TransformationMode.SmoothTransformation)
                logo_label.setPixmap(scaled)
                logo_label.setFixedSize(scaled.width(), scaled.height())
            else:
                logo_label.setText("RITE-HITE")
                logo_label.setObjectName("brandFallback")
        else:
            logo_label.setText("RITE-HITE")
            logo_label.setObjectName("brandFallback")
        content_layout.addWidget(logo_label, 0, Qt.AlignVCenter)

        divider = QFrame()
        divider.setObjectName("mastheadDivider")
        divider.setFixedWidth(1)
        divider.setFixedHeight(56)
        content_layout.addWidget(divider, 0, Qt.AlignVCenter)

        text_column = QVBoxLayout()
        text_column.setSpacing(3)
        content_layout.addLayout(text_column, 1)

        self._eyebrow = QLabel()
        self._eyebrow.setObjectName("eyebrowLabel")
        text_column.addWidget(self._eyebrow)

        self._hero_title = QLabel()
        self._hero_title.setObjectName("heroTitle")
        text_column.addWidget(self._hero_title)

        self._hero_subtitle = QLabel()
        self._hero_subtitle.setObjectName("heroSubtitle")
        self._hero_subtitle.setWordWrap(True)
        text_column.addWidget(self._hero_subtitle)

        outer.addWidget(content)

        accent_line = QFrame()
        accent_line.setObjectName("accentLine")
        accent_line.setFixedHeight(3)
        outer.addWidget(accent_line)

        return masthead

    def _build_file_explorer_tab(self) -> QWidget:
        tab = QWidget()
        tab.setObjectName("fileExplorerTab")
        layout = QVBoxLayout(tab)
        layout.setSpacing(14)
        layout.setContentsMargins(8, 10, 8, 10)

        actions = QHBoxLayout()
        self.refresh_history_button = QPushButton()
        self.refresh_history_button.setObjectName("utilityButton")
        self.refresh_history_button.setIcon(qta_icon("fa6s.rotate", "ink.secondary"))
        self.refresh_history_button.setToolTip("Refresh file list (F5)")
        self.refresh_history_button.clicked.connect(self.refresh_history)
        actions.addWidget(self.refresh_history_button)

        self.open_folder_button = QPushButton()
        self.open_folder_button.setObjectName("utilityButton")
        self.open_folder_button.setIcon(qta_icon("fa6s.folder-open", "ink.secondary"))
        self.open_folder_button.setToolTip("Open exports folder (Ctrl+O)")
        self.open_folder_button.clicked.connect(self.open_exports_folder)
        actions.addWidget(self.open_folder_button)

        self.change_folder_button = QPushButton()
        self.change_folder_button.setObjectName("utilityButton")
        self.change_folder_button.setIcon(qta_icon("fa6s.folder-tree", "ink.secondary"))
        self.change_folder_button.setToolTip("Choose a different folder for exports")
        self.change_folder_button.clicked.connect(self._change_exports_folder)
        actions.addWidget(self.change_folder_button)

        self.reset_folder_button = QPushButton()
        self.reset_folder_button.setObjectName("utilityButton")
        self.reset_folder_button.setIcon(qta_icon("fa6s.arrow-rotate-left", "ink.secondary"))
        self.reset_folder_button.setToolTip("Revert to the default exports folder")
        self.reset_folder_button.clicked.connect(self._reset_exports_folder)
        actions.addWidget(self.reset_folder_button)

        actions.addStretch(1)

        layout.addLayout(actions)

        self.exports_path_label = QLabel(str(get_exports_dir()))
        self.exports_path_label.setObjectName("exportsPathLabel")
        self.exports_path_label.setWordWrap(True)
        layout.addWidget(self.exports_path_label)

        filters = QHBoxLayout()
        filters.setSpacing(10)

        self.history_search = QLineEdit()
        self.history_search.setObjectName("historySearchField")
        self.history_search.setPlaceholderText("Search by filename…")
        self.history_search.setClearButtonEnabled(True)
        self.history_search.textChanged.connect(self._on_history_filters_changed)
        filters.addWidget(self.history_search, 2)

        self._outcome_label = QLabel()
        self._outcome_label.setObjectName("filterFieldLabel")
        filters.addWidget(self._outcome_label, 0, Qt.AlignRight | Qt.AlignVCenter)

        self.history_outcome_filter = QComboBox()
        self.history_outcome_filter.setObjectName("historyOutcomeFilter")
        self.history_outcome_filter.addItems(["All", "PASS", "FAIL", "UNKNOWN"])
        self.history_outcome_filter.currentIndexChanged.connect(self._on_history_filters_changed)
        filters.addWidget(self.history_outcome_filter, 0)

        self.clear_history_filters_button = QPushButton()
        self.clear_history_filters_button.setObjectName("utilityButton")
        self.clear_history_filters_button.clicked.connect(self._clear_history_filters)
        filters.addWidget(self.clear_history_filters_button)

        layout.addLayout(filters)

        self.history_stack = QStackedWidget()
        self.history_stack.setObjectName("historyStack")

        self.history_table = QTableWidget(0, 4)
        self.history_table.setObjectName("historyTable")
        self.history_table.setHorizontalHeaderLabels(["Filename", "Saved Time", "Outcome", ""])
        self.history_table.horizontalHeader().setSectionResizeMode(0, QHeaderView.Stretch)
        self.history_table.horizontalHeader().setSectionResizeMode(1, QHeaderView.ResizeToContents)
        self.history_table.horizontalHeader().setSectionResizeMode(2, QHeaderView.ResizeToContents)
        self.history_table.horizontalHeader().setSectionResizeMode(3, QHeaderView.Fixed)
        self.history_table.setColumnWidth(3, 52)
        self.history_table.setSelectionBehavior(QTableWidget.SelectRows)
        self.history_table.setEditTriggers(QTableWidget.NoEditTriggers)
        self.history_table.setAlternatingRowColors(True)
        self.history_table.setShowGrid(False)
        self.history_table.verticalHeader().setVisible(False)
        self.history_table.setWordWrap(False)
        self.history_table.setSortingEnabled(True)
        self.history_table.cellClicked.connect(self._on_history_cell_clicked)
        self.history_table.cellDoubleClicked.connect(self.open_selected_export)
        self.history_table.setContextMenuPolicy(Qt.CustomContextMenu)
        self.history_table.customContextMenuRequested.connect(self._show_history_context_menu)

        self.empty_history_state = QWidget()
        self.empty_history_state.setObjectName("emptyHistoryState")
        empty_layout = QVBoxLayout(self.empty_history_state)
        empty_layout.setAlignment(Qt.AlignCenter)
        empty_layout.setSpacing(16)

        self.empty_history_icon = QLabel()
        self.empty_history_icon.setObjectName("emptyHistoryIcon")
        self.empty_history_icon.setAlignment(Qt.AlignCenter)
        self.empty_history_icon.setPixmap(
            qta_icon("fa6s.inbox", "ink.muted").pixmap(56, 56)
        )
        empty_layout.addWidget(self.empty_history_icon)

        self.empty_history_title = QLabel()
        self.empty_history_title.setObjectName("emptyHistoryTitle")
        self.empty_history_title.setAlignment(Qt.AlignCenter)
        empty_layout.addWidget(self.empty_history_title)

        self.empty_history_hint = QLabel()
        self.empty_history_hint.setObjectName("emptyHistoryHint")
        self.empty_history_hint.setAlignment(Qt.AlignCenter)
        self.empty_history_hint.setWordWrap(True)
        empty_layout.addWidget(self.empty_history_hint)

        empty_buttons = QHBoxLayout()
        empty_buttons.setAlignment(Qt.AlignCenter)
        self.empty_go_device_tools = QPushButton()
        self.empty_go_device_tools.setObjectName("utilityButton")
        self.empty_go_device_tools.clicked.connect(
            lambda: self.sidebar.set_active(PAGE_INDEX_DEVICE_TOOLS)
        )
        empty_buttons.addWidget(self.empty_go_device_tools)

        self.empty_refresh = QPushButton()
        self.empty_refresh.setObjectName("utilityButton")
        self.empty_refresh.clicked.connect(self.refresh_history)
        empty_buttons.addWidget(self.empty_refresh)
        empty_layout.addLayout(empty_buttons)

        self.history_stack.addWidget(self.history_table)
        self.history_stack.addWidget(self.empty_history_state)
        self.history_stack.setCurrentIndex(0)

        layout.addWidget(self.history_stack, 1)

        return tab

    def _build_export_tab(self) -> QWidget:
        tab = QWidget()
        tab.setObjectName("exportTab")
        layout = QVBoxLayout(tab)
        layout.setSpacing(20)
        layout.setContentsMargins(8, 10, 8, 10)

        self.workflow_banner = QLabel()
        self.workflow_banner.setObjectName("workflowBanner")
        self.workflow_banner.setWordWrap(True)
        layout.addWidget(self.workflow_banner)

        meta_row = QHBoxLayout()
        meta_row.setSpacing(14)

        connection_group = QGroupBox()
        connection_group.setObjectName("connectionReadinessCard")
        connection_layout = QVBoxLayout(connection_group)
        connection_layout.setContentsMargins(14, 12, 14, 12)
        connection_layout.setSpacing(8)

        self._connection_heading = QLabel()
        self._connection_heading.setObjectName("cardTitle")
        connection_layout.addWidget(self._connection_heading)

        self.nucleo_connection_label = QLabel()
        self.nucleo_connection_label.setWordWrap(True)
        connection_layout.addWidget(self.nucleo_connection_label)

        self.esp_connection_label = QLabel()
        self.esp_connection_label.setWordWrap(True)
        connection_layout.addWidget(self.esp_connection_label)

        self.connection_links = QLabel(
            '<a href="https://www.st.com/en/development-tools/stsw-link009.html">Test Fixture Drivers (ST)</a>'
            " &nbsp;·&nbsp; "
            '<a href="https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/index.html">Rite-Hite Connect Module Info (Espressif)</a>'
        )
        self.connection_links.setObjectName("connectionTroubleshootLinks")
        self.connection_links.setOpenExternalLinks(True)
        self.connection_links.setWordWrap(True)
        connection_layout.addWidget(self.connection_links)

        last_group = QGroupBox()
        last_group.setObjectName("lastOperationCard")
        last_layout = QVBoxLayout(last_group)
        last_layout.setContentsMargins(14, 12, 14, 12)
        last_layout.setSpacing(6)

        self._last_heading = QLabel()
        self._last_heading.setObjectName("cardTitle")
        last_layout.addWidget(self._last_heading)

        self.last_operation_body = QLabel()
        self.last_operation_body.setObjectName("lastOperationBody")
        self.last_operation_body.setWordWrap(True)
        self.last_operation_body.setAlignment(Qt.AlignTop | Qt.AlignLeft)
        self.last_operation_body.setSizePolicy(QSizePolicy.Policy.Preferred, QSizePolicy.Policy.MinimumExpanding)
        self.last_operation_body.setMinimumHeight(72)
        last_layout.addWidget(self.last_operation_body)

        meta_row.addWidget(connection_group, 3)
        meta_row.addWidget(last_group, 2)
        layout.addLayout(meta_row)

        cards = QHBoxLayout()
        cards.setSpacing(14)

        export_group = QGroupBox()
        export_group.setObjectName("primaryCard")
        export_layout = QVBoxLayout(export_group)
        export_layout.setContentsMargins(16, 14, 16, 14)
        export_layout.setSpacing(10)

        self._export_card_title = QLabel()
        self._export_card_title.setObjectName("cardTitle")
        export_layout.addWidget(self._export_card_title)

        self._export_card_subtitle = QLabel()
        self._export_card_subtitle.setObjectName("cardSubtitle")
        self._export_card_subtitle.setWordWrap(True)
        export_layout.addWidget(self._export_card_subtitle)

        export_layout.addSpacing(6)

        self.extract_button = QPushButton()
        self.extract_button.setObjectName("primaryExportButton")
        self.extract_button.setIcon(qta_icon("fa6s.download", "ink.inverse"))
        self.extract_button.setToolTip(
            "Extract Data from Test Fixture (Ctrl+E) — Uses the test fixture serial connection. Export is available only after the fixture reaches a terminal pass or fail state."
        )
        self.extract_button.clicked.connect(self.handle_extract_clicked)
        export_layout.addWidget(self.extract_button)
        export_layout.addStretch(1)
        export_group.setMinimumHeight(140)

        esp32_card = self._build_esp32_test_card()
        placeholder_card = self._build_production_firmware_card()

        for card in (export_group, esp32_card, placeholder_card):
            self._apply_card_shadow(card)

        cards.addWidget(export_group, 3)
        cards.addWidget(esp32_card, 2)
        cards.addWidget(placeholder_card, 2)

        layout.addLayout(cards)

        layout.addStretch(1)
        return tab

    def _build_esp32_test_card(self) -> QGroupBox:
        group = QGroupBox()
        group.setObjectName("secondaryCard")
        group.setMinimumHeight(140)
        layout = QVBoxLayout(group)
        layout.setContentsMargins(14, 14, 14, 14)
        layout.setSpacing(10)

        self._test_fw_title = QLabel()
        self._test_fw_title.setObjectName("cardTitle")
        layout.addWidget(self._test_fw_title)

        self._test_fw_subtitle = QLabel()
        self._test_fw_subtitle.setObjectName("cardSubtitle")
        self._test_fw_subtitle.setWordWrap(True)
        layout.addWidget(self._test_fw_subtitle)

        layout.addStretch(1)

        self.upload_test_button = QPushButton()
        self.upload_test_button.setObjectName("secondaryActionButton")
        self.upload_test_button.setIcon(qta_icon("fa6s.bolt", "accent.testBlueInk"))
        self.upload_test_button.setToolTip(
            "Upload Test Firmware (Ctrl+U) — Uploads the fixed EOL test firmware package to the Rite-Hite Connect module through the programmer."
        )
        self.upload_test_button.clicked.connect(self.handle_upload_test_clicked)
        layout.addWidget(self.upload_test_button)

        return group

    def _build_production_firmware_card(self) -> QGroupBox:
        group = QGroupBox()
        group.setObjectName("secondaryCard")
        group.setMinimumHeight(140)
        layout = QVBoxLayout(group)
        layout.setContentsMargins(14, 14, 14, 14)
        layout.setSpacing(10)

        self._prod_fw_title = QLabel()
        self._prod_fw_title.setObjectName("cardTitle")
        layout.addWidget(self._prod_fw_title)

        self._prod_fw_subtitle = QLabel()
        self._prod_fw_subtitle.setObjectName("cardSubtitle")
        self._prod_fw_subtitle.setWordWrap(True)
        layout.addWidget(self._prod_fw_subtitle)

        layout.addStretch(1)

        self.upload_production_button = QPushButton()
        self.upload_production_button.setObjectName("secondaryActionButton")
        self.upload_production_button.setIcon(
            qta_icon("fa6s.circle-up", "accent.prodGreenInk")
        )
        self.upload_production_button.setToolTip("Upload production firmware to the Rite-Hite Connect module")
        self.upload_production_button.clicked.connect(self._handle_production_upload_clicked)
        layout.addWidget(self.upload_production_button)

        return group

    def _handle_production_upload_clicked(self) -> None:
        QMessageBox.information(
            self,
            tr("coming_soon"),
            tr("coming_soon_body"),
        )

    # ------------------------------------------------------------------
    #  One Click Test tab
    # ------------------------------------------------------------------
    def _build_one_click_tab(self) -> QWidget:
        tab = QWidget()
        tab.setObjectName("oneClickTab")
        outer = QVBoxLayout(tab)
        outer.setContentsMargins(16, 16, 16, 16)
        outer.setSpacing(14)

        # --- Connection readiness (duplicated for independence) ---
        conn_card = QGroupBox()
        conn_card.setObjectName("secondaryCard")
        conn_layout = QVBoxLayout(conn_card)
        conn_layout.setContentsMargins(14, 14, 14, 14)
        conn_layout.setSpacing(6)

        self._oct_conn_title = QLabel()
        self._oct_conn_title.setObjectName("cardTitle")
        conn_layout.addWidget(self._oct_conn_title)

        self._oct_nucleo_lbl = QLabel()
        self._oct_nucleo_lbl.setObjectName("connItemLabel")
        conn_layout.addWidget(self._oct_nucleo_lbl)

        self._oct_esp_lbl = QLabel()
        self._oct_esp_lbl.setObjectName("connItemLabel")
        conn_layout.addWidget(self._oct_esp_lbl)

        outer.addWidget(conn_card)

        # --- Action card ---
        action_card = QGroupBox()
        action_card.setObjectName("primaryCard")
        action_layout = QVBoxLayout(action_card)
        action_layout.setContentsMargins(14, 14, 14, 14)
        action_layout.setSpacing(10)

        self._oct_title = QLabel()
        self._oct_title.setObjectName("cardTitle")
        action_layout.addWidget(self._oct_title)

        self._oct_subtitle = QLabel()
        self._oct_subtitle.setObjectName("cardSubtitle")
        self._oct_subtitle.setWordWrap(True)
        action_layout.addWidget(self._oct_subtitle)

        self.oct_start_btn = QPushButton()
        self.oct_start_btn.setObjectName("primaryExportButton")
        self.oct_start_btn.setIcon(qta_icon("fa6s.circle-play", "ink.inverse"))
        self.oct_start_btn.clicked.connect(self._handle_one_click_start)
        action_layout.addWidget(self.oct_start_btn)

        outer.addWidget(action_card)

        # --- Step progress rail ---
        stepper_card = QGroupBox()
        stepper_card.setObjectName("secondaryCard")
        stepper_card.setMinimumHeight(140)
        stepper_layout = QVBoxLayout(stepper_card)
        stepper_layout.setContentsMargins(16, 14, 16, 14)
        stepper_layout.setSpacing(6)

        self._oct_step_keys = [
            "oct_step_upload",
            "oct_step_comms",
            "oct_step_digital_inputs",
            "oct_step_analog_inputs",
            "oct_step_digital_outputs",
            "oct_step_relay_outputs",
            "oct_step_buttons",
            "oct_step_leds",
            "oct_step_extract",
        ]

        self._oct_step_progress = StepProgressBar()
        self._oct_step_progress.set_steps([tr(k) for k in self._oct_step_keys])
        self._oct_step_progress.set_reduced_motion(
            bool(self._app_settings.get("reduced_motion", False))
        )
        stepper_layout.addWidget(self._oct_step_progress)

        outer.addWidget(stepper_card)

        # --- Detail / log label ---
        self._oct_detail = QLabel()
        self._oct_detail.setObjectName("cardSubtitle")
        self._oct_detail.setWordWrap(True)
        self._oct_detail.setAlignment(Qt.AlignCenter)
        self._oct_detail.setVisible(False)
        outer.addWidget(self._oct_detail)

        outer.addStretch(1)
        return tab

    def _refresh_oct_connection(self) -> None:
        """Update the One Click tab's connection readiness labels."""
        try:
            available = list_available_ports()
        except Exception:
            available = []

        nucleo_ports = [p for p in available if looks_like_stlink_port(p)]
        if nucleo_ports:
            self._oct_nucleo_lbl.setText(
                "\u2713  " + tr("fixture_detected", port=nucleo_ports[0].device)
            )
            set_status_pill(self._oct_nucleo_lbl, "success")
        elif not available:
            self._oct_nucleo_lbl.setText("\u26A0  " + tr("fixture_no_ports"))
            set_status_pill(self._oct_nucleo_lbl, "error")
        else:
            self._oct_nucleo_lbl.setText("\u26A0  " + tr("fixture_not_found"))
            set_status_pill(self._oct_nucleo_lbl, "warn")

        esp_ok, esp_msg = describe_esp_prog_detection()
        if esp_ok:
            self._oct_esp_lbl.setText("\u2713  " + esp_msg)
            set_status_pill(self._oct_esp_lbl, "success")
        else:
            self._oct_esp_lbl.setText("\u26A0  " + esp_msg)
            set_status_pill(self._oct_esp_lbl, "error")

    def _is_one_click_running(self) -> bool:
        return self.oneclick_thread is not None

    def _set_one_click_busy(self, busy: bool) -> None:
        self.oct_start_btn.setEnabled(not busy)
        self.extract_button.setEnabled(not busy)
        self.upload_test_button.setEnabled(not busy)
        self.upload_production_button.setEnabled(not busy)

    def _warn_one_click_busy(self) -> None:
        QMessageBox.warning(self, tr("oct_title"), tr("oct_busy_locked"))

    def _handle_one_click_start(self) -> None:
        if self.oneclick_thread is not None:
            return
        if self.upload_thread is not None:
            QMessageBox.warning(self, tr("oct_title"), tr("oct_upload_busy"))
            return

        try:
            config = get_default_upload_config()
        except Esp32UploadError as exc:
            QMessageBox.critical(self, tr("oct_title"), str(exc))
            return

        discovery = discover_upload_targets()
        if discovery.auto_choice is None:
            reasons = "\n".join(f"  \u2022 {r}" for r in discovery.failure_reasons)
            QMessageBox.critical(self, tr("oct_title"), tr("oct_no_esp") + "\n" + reasons)
            return

        try:
            available = list_available_ports()
        except Exception as exc:
            QMessageBox.critical(self, tr("oct_title"), str(exc))
            return

        nucleo_ports = [p for p in available if looks_like_stlink_port(p)]
        if not nucleo_ports:
            QMessageBox.critical(self, tr("oct_title"), tr("fixture_not_found"))
            return

        fixture_port = nucleo_ports[0].device

        self._oct_reset_stepper()
        self._set_one_click_busy(True)
        self.log_drawer.clear()

        worker = OneClickWorker(config, discovery.auto_choice, fixture_port)
        thread = QThread(self)
        worker.moveToThread(thread)
        thread.started.connect(worker.run)
        worker.stage_changed.connect(self._oct_on_stage_changed)
        worker.log_message.connect(self._oct_on_log)
        worker.succeeded.connect(self._oct_on_success)
        worker.failed.connect(self._oct_on_failure)
        worker.finished.connect(thread.quit)
        worker.finished.connect(worker.deleteLater)
        thread.finished.connect(thread.deleteLater)
        thread.finished.connect(self._oct_cleanup)

        self.oneclick_worker = worker
        self.oneclick_thread = thread
        thread.start()

    def _oct_reset_stepper(self) -> None:
        self._oct_step_progress.reset()

    def _oct_on_stage_changed(self, stage_idx: int, message: str) -> None:
        self._oct_step_progress.set_active(stage_idx)
        _ = message

    def _oct_on_log(self, msg: str) -> None:
        self.append_status_message(msg)

    def _oct_on_success(self, msg: str) -> None:
        self._oct_step_progress.mark_all_done()
        self.append_status_message(msg, severity="success")
        self._show_flash_banner(msg, success=True)
        self.refresh_history()

    def _oct_on_failure(self, msg: str) -> None:
        current = self._oct_step_progress.state_at(
            max(0, self.pages.currentIndex())
        )
        for i in range(len(self._oct_step_keys)):
            if self._oct_step_progress.state_at(i) == StepState.ACTIVE:
                self._oct_step_progress.mark_failed(i)
                break
        else:
            _ = current
        self.append_status_message(msg, severity="error")
        self._show_flash_banner(msg, success=False)

    def _oct_cleanup(self) -> None:
        self.oneclick_thread = None
        self.oneclick_worker = None
        self._set_one_click_busy(False)

    def _apply_card_shadow(self, widget: QWidget) -> None:
        apply_shadow(widget, "md")
        widget.installEventFilter(self)

    def changeEvent(self, event) -> None:  # noqa: ANN001 - Qt event
        super().changeEvent(event)
        if event.type() == QEvent.Type.WindowStateChange:
            self._schedule_layout_refresh()

    def eventFilter(self, obj: QObject, event: QEvent) -> bool:
        if isinstance(obj, QWidget) and isinstance(
            obj.graphicsEffect(), QGraphicsDropShadowEffect
        ):
            if event.type() == QEvent.Type.Enter:
                tune_shadow(obj, "lg")
            elif event.type() == QEvent.Type.Leave:
                tune_shadow(obj, "md")
        return super().eventFilter(obj, event)

    def _apply_styles(self) -> None:
        self.setStyleSheet(build_stylesheet())

    def _apply_dark_styles(self) -> None:
        self.setStyleSheet(build_dark_stylesheet())

    def set_status_message(self, message: str, severity: str = "info") -> None:
        self.log_drawer.set_message(message, severity)

    def append_status_message(self, message: str, severity: str = "info") -> None:
        self.log_drawer.append(message, severity)

    def _handle_escape_window_mode(self) -> None:
        if self.isFullScreen():
            self.showMaximized()
            self._schedule_layout_refresh()
        elif self.isMaximized():
            self.showNormal()
            self._schedule_layout_refresh()

    def _schedule_layout_refresh(self, *_args) -> None:
        QTimer.singleShot(0, self._refresh_shell_layout)

    def _refresh_shell_layout(self) -> None:
        central = self.centralWidget()
        if central is not None:
            layout = central.layout()
            if layout is not None:
                layout.invalidate()
                layout.activate()

        for widget in (self.pages, self.progress_bar, self.log_drawer, self.sidebar):
            widget.updateGeometry()
        self.updateGeometry()

    def _show_progress(self) -> None:
        self.progress_bar.setVisible(True)
        self._schedule_layout_refresh()

    def _hide_progress(self) -> None:
        self.progress_bar.setVisible(False)
        self._schedule_layout_refresh()

    def handle_extract_clicked(self) -> None:
        if self._is_one_click_running():
            self._warn_one_click_busy()
            return

        if self._app_settings.get("confirm_before_extract", True):
            reply = QMessageBox.question(
                self,
                tr("confirm_extract_title"),
                tr("confirm_extract_body"),
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.Cancel,
                QMessageBox.StandardButton.Yes,
            )
            if reply != QMessageBox.StandardButton.Yes:
                return

        self.extract_button.setEnabled(False)
        self._show_progress()
        QApplication.setOverrideCursor(Qt.WaitCursor)

        try:
            selected_port = self.resolve_serial_port()
            if selected_port is None:
                self.set_status_message(tr("export_cancelled"))
                return

            self.set_status_message(tr("export_connecting", port=selected_port.device))
            QApplication.processEvents()

            export_run = self.client.export_latest_run(selected_port.device)
            saved_path = write_export_csv(export_run)
            save_last_successful_export(saved_path, selected_port.device)
            self._update_last_operation_ui()
            self.append_status_message(
                tr("export_saved", seq=export_run.run_sequence, outcome=export_run.outcome, name=saved_path.name),
                severity="success",
            )
            self._show_flash_banner(tr("export_saved_flash", name=saved_path.name), success=True)
            self.refresh_history()
            self.sidebar.set_active(PAGE_INDEX_FILE_EXPLORER)
        except NoPortsAvailableError as exc:
            self.set_status_message(str(exc), severity="error")
            self._show_flash_banner(tr("export_failed_flash", reason=exc), success=False)
        except DeviceEmptyError as exc:
            self.set_status_message(str(exc), severity="error")
            self._show_flash_banner(tr("export_failed_flash", reason=exc), success=False)
        except DeviceRunningError as exc:
            self.set_status_message(str(exc), severity="error")
            self._show_flash_banner(tr("export_failed_flash", reason=exc), success=False)
        except DeviceAlreadyExportedError as exc:
            self.set_status_message(tr("export_firmware_blocked"), severity="error")
            self._show_flash_banner(tr("export_firmware_blocked_flash"), success=False)
        except Exception as exc:
            QMessageBox.critical(self, tr("export_failed_title"), str(exc))
            self.set_status_message(tr("export_failed_flash", reason=exc), severity="error")
            self._show_flash_banner(tr("export_failed_flash", reason=exc), success=False)
        finally:
            self._hide_progress()
            QApplication.restoreOverrideCursor()
            self.extract_button.setEnabled(True)

    def resolve_serial_port(self) -> PortChoice | None:
        try:
            return detect_preferred_port()
        except PortSelectionRequiredError as exc:
            dialog = PortPickerDialog(exc.ports, exc.reason, self)
            if dialog.exec() == QDialog.Accepted:
                return dialog.selected_port()
            return None

    def handle_upload_test_clicked(self) -> None:
        if self._is_one_click_running():
            self._warn_one_click_busy()
            return

        if self.upload_thread is not None:
            return

        try:
            config = get_default_upload_config()
            selection = self.resolve_upload_target(config)
            if selection is None:
                self.set_status_message(tr("upload_cancelled"))
                return

            detected_ports = " / ".join(selection.pair_ports)
            if selection.is_pair:
                self.set_status_message(
                    f"Starting Rite-Hite Connect test firmware upload. Board: {config.board_name}. Paired ports: {detected_ports}. Uploading on {selection.upload_port}."
                )
            else:
                self.set_status_message(
                    f"Starting Rite-Hite Connect test firmware upload. Board: {config.board_name}. Manual upload port: {selection.upload_port}."
                )

            self.start_upload_worker(config, selection)
        except Esp32UploadError as exc:
            self.show_upload_error_dialog(str(exc))
            self.set_status_message(str(exc), severity="error")
        except Exception as exc:
            self.show_upload_error_dialog(str(exc))
            self.set_status_message(tr("upload_setup_failed", reason=exc), severity="error")

    def resolve_upload_target(self, config: Esp32UploadConfig) -> Esp32UploadChoice | None:
        discovery = discover_upload_targets()
        if discovery.auto_choice is not None:
            if self.confirm_upload_choice(config, discovery.auto_choice):
                return discovery.auto_choice
            return None

        if not discovery.manual_choices:
            raise Esp32UploadError(tr("no_programmer_detected"))

        dialog = UploadTargetPickerDialog(discovery.manual_choices, discovery.reason or "Select the Rite-Hite Connect upload port.", self)
        if dialog.exec() == QDialog.Accepted:
            return dialog.selected_choice()
        return None

    def confirm_upload_choice(self, config: Esp32UploadConfig, choice: Esp32UploadChoice) -> bool:
        dialog = UploadConfirmDialog(config, choice, self)
        return dialog.exec() == QDialog.Accepted

    def show_upload_error_dialog(self, message: str) -> None:
        dialog = UploadErrorDialog(message, self)
        dialog.exec()

    def start_upload_worker(self, config: Esp32UploadConfig, selection: Esp32UploadChoice) -> None:
        self.upload_test_button.setEnabled(False)
        self._last_upload_choice = selection
        self._show_progress()

        self.upload_thread = QThread(self)
        self.upload_worker = Esp32UploadWorker(config, selection)
        self.upload_worker.moveToThread(self.upload_thread)

        self.upload_thread.started.connect(self.upload_worker.run)
        self.upload_worker.log_message.connect(self.append_status_message)
        self.upload_worker.upload_succeeded.connect(self.on_upload_succeeded)
        self.upload_worker.upload_failed.connect(self.on_upload_failed)
        self.upload_worker.finished.connect(self.upload_thread.quit)
        self.upload_worker.finished.connect(self.upload_worker.deleteLater)
        self.upload_thread.finished.connect(self.upload_thread.deleteLater)
        self.upload_thread.finished.connect(self.on_upload_thread_finished)

        self.upload_thread.start()

    def on_upload_succeeded(self, message: str) -> None:
        self.append_status_message(message, severity="success")
        self._show_flash_banner(tr("upload_success_flash"), success=True)
        if self._last_upload_choice is not None:
            save_last_successful_upload(self._last_upload_choice.upload_port, message.strip())
            self._update_last_operation_ui()

    def on_upload_failed(self, message: str) -> None:
        self.append_status_message(tr("upload_failed_log", reason=message), severity="error")
        self._show_flash_banner(tr("upload_failed_flash", reason=message), success=False)
        self.show_upload_error_dialog(message)

    def on_upload_thread_finished(self) -> None:
        self._hide_progress()
        self.upload_test_button.setEnabled(True)
        self.upload_thread = None
        self.upload_worker = None
        self._last_upload_choice = None

    def _show_flash_banner(self, message: str, success: bool = True) -> None:
        self.flash_banner.setProperty("severity", "success" if success else "error")
        self.flash_banner.setStyleSheet("")
        self.flash_banner.style().unpolish(self.flash_banner)
        self.flash_banner.style().polish(self.flash_banner)
        self.flash_banner.setText(message)
        self.flash_banner.setVisible(True)
        self._schedule_layout_refresh()

        def _hide_banner() -> None:
            self.flash_banner.setVisible(False)
            self._schedule_layout_refresh()

        QTimer.singleShot(5000, _hide_banner)

    def _update_workflow_banner(self) -> None:
        ports = list_available_ports()
        stlink_ports = [p for p in ports if looks_like_stlink_port(p)]
        fixture_ok = len(stlink_ports) > 0
        esp_ok, _ = describe_esp_prog_detection()

        if not fixture_ok and not esp_ok:
            text = tr("wf_connect_fixture")
            icon = "\u26A0\uFE0F"
        elif fixture_ok and not esp_ok:
            text = tr("wf_fixture_ready")
            icon = "\U0001F50C"
        else:
            text = tr("wf_all_ready")
            icon = "\u2705"

        self.workflow_banner.setText(f"{icon}  {text}")

    def _on_main_page_changed(self, index: int) -> None:
        self.pages.setCurrentIndex(index)
        if index == PAGE_INDEX_DEVICE_TOOLS:
            self._refresh_connection_status()
            self.connection_timer.start()
        elif index == PAGE_INDEX_ONE_CLICK:
            self._refresh_oct_connection()
            self.connection_timer.start()
        else:
            self.connection_timer.stop()
            if self._app_settings.get("auto_refresh_on_tab_switch", True):
                self.refresh_history()

    def _on_connection_timer(self) -> None:
        idx = self.pages.currentIndex()
        if idx == PAGE_INDEX_DEVICE_TOOLS:
            self._refresh_connection_status()
        elif idx == PAGE_INDEX_ONE_CLICK:
            self._refresh_oct_connection()

    def _refresh_connection_status(self) -> None:
        ports = list_available_ports()
        stlink_ports = [p for p in ports if looks_like_stlink_port(p)]
        if not ports:
            self._set_connection_line(
                self.nucleo_connection_label, False, tr("fixture_no_ports"),
            )
        elif not stlink_ports:
            self._set_connection_line(
                self.nucleo_connection_label, False, tr("fixture_not_detected"),
            )
        elif len(stlink_ports) == 1:
            self._set_connection_line(
                self.nucleo_connection_label, True,
                tr("fixture_detected", port=stlink_ports[0].device),
            )
        else:
            self._set_connection_line(
                self.nucleo_connection_label, True,
                tr("fixture_multiple", count=len(stlink_ports)),
            )

        esp_ok, esp_message = describe_esp_prog_detection()
        self._set_connection_line(self.esp_connection_label, esp_ok, tr("module_prefix", message=esp_message))
        self._update_workflow_banner()

    def _set_connection_line(self, label: QLabel, ok: bool, text: str) -> None:
        set_status_pill(label, "success" if ok else "error")
        icon_char = "\u2713" if ok else "\u26A0"
        label.setText(f"{icon_char}  {text}")

    def _update_last_operation_ui(self) -> None:
        record = load_last_operation()
        if record is None:
            self.last_operation_body.setText(tr("no_operation_recorded"))
            self.last_operation_body.setStyleSheet(
                f"color: {token_color('ink.tertiary')}; font-size: 13px; line-height: 1.5;"
                " font-style: italic;"
            )
            return
        self.last_operation_body.setStyleSheet(
            f"color: {token_color('ink.primary')}; font-size: 13px; line-height: 1.5;"
        )
        self.last_operation_body.setText(record.summarize())

    def _on_history_filters_changed(self, *_args: object) -> None:
        self._update_history_table_rows()

    def _clear_history_filters(self) -> None:
        self.history_search.blockSignals(True)
        self.history_outcome_filter.blockSignals(True)
        self.history_search.clear()
        self.history_outcome_filter.setCurrentIndex(0)
        self.history_search.blockSignals(False)
        self.history_outcome_filter.blockSignals(False)
        self._update_history_table_rows()

    def _collect_filtered_entries(self) -> list[ExportHistoryEntry]:
        needle = self.history_search.text().strip().lower()
        outcome = self.history_outcome_filter.currentText()
        rows: list[ExportHistoryEntry] = []
        for entry in self._all_history_entries:
            if needle and needle not in entry.filename.lower():
                continue
            if outcome != "All" and entry.outcome != outcome:
                continue
            rows.append(entry)
        return rows

    def _update_empty_state_panel(self, any_on_disk: bool, filter_active: bool) -> None:
        if not any_on_disk:
            self.empty_history_title.setText(tr("no_exports_yet"))
            self.empty_history_hint.setText(tr("no_exports_hint"))
            self.empty_go_device_tools.setVisible(True)
            self.empty_refresh.setVisible(True)
        elif filter_active:
            self.empty_history_title.setText(tr("no_files_match"))
            self.empty_history_hint.setText(tr("no_files_match_hint"))
            self.empty_go_device_tools.setVisible(False)
            self.empty_refresh.setVisible(True)
        else:
            self.empty_history_title.setText(tr("no_rows"))
            self.empty_history_hint.setText("")
            self.empty_go_device_tools.setVisible(False)
            self.empty_refresh.setVisible(True)

    def _update_history_table_rows(self) -> None:
        filtered = self._collect_filtered_entries()
        any_on_disk = len(self._all_history_entries) > 0
        filter_active = bool(self.history_search.text().strip()) or self.history_outcome_filter.currentIndex() != 0

        if not filtered:
            self.history_stack.setCurrentIndex(1)
            self._update_empty_state_panel(any_on_disk, filter_active and any_on_disk)
        else:
            self.history_stack.setCurrentIndex(0)

        self.history_table.setSortingEnabled(False)
        self.history_table.setRowCount(len(filtered))
        for row_index, entry in enumerate(filtered):
            filename_item = QTableWidgetItem(entry.filename)
            filename_item.setData(Qt.UserRole, str(entry.path))
            saved_item = QTableWidgetItem(entry.saved_time)
            outcome_item = QTableWidgetItem(entry.outcome)
            outcome_item.setTextAlignment(Qt.AlignCenter)
            self._apply_outcome_badge(outcome_item, entry.outcome)
            self.history_table.setItem(row_index, 0, filename_item)
            self.history_table.setItem(row_index, 1, saved_item)
            self.history_table.setItem(row_index, 2, outcome_item)
            self.history_table.setCellWidget(row_index, 3, self._create_delete_cell_widget(entry.path))

        self.history_table.resizeRowsToContents()
        self.history_table.setColumnWidth(2, 120)
        self.history_table.setColumnWidth(3, 52)
        self.history_table.setSortingEnabled(True)

    def _create_delete_cell_widget(self, path: Path) -> QWidget:
        wrapper = QWidget()
        wrapper.setStyleSheet("background: transparent;")
        layout = QHBoxLayout(wrapper)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setAlignment(Qt.AlignCenter)
        btn = QPushButton()
        btn.setIcon(qta_icon("fa6s.trash-can", "ink.tertiary"))
        btn.setToolTip(tr("tooltip_delete"))
        btn.setFixedSize(32, 32)
        btn.setCursor(Qt.PointingHandCursor)
        btn.setStyleSheet(
            "QPushButton { background: transparent; border: none; min-height: 0; padding: 0; }"
            f"QPushButton:hover {{ background: {token_color('status.errorBg')};"
            f" border: 1px solid {token_color('status.errorBorder')}; border-radius: 6px; }}"
        )
        btn.clicked.connect(lambda checked=False, p=path: self._confirm_delete_export(p))
        layout.addWidget(btn)
        return wrapper

    def _on_history_cell_clicked(self, row: int, column: int) -> None:
        if column != 3:
            return
        item = self.history_table.item(row, 3)
        if item is None:
            return
        path_str = item.data(Qt.UserRole)
        if not path_str:
            return
        self._confirm_delete_export(Path(path_str))

    def _confirm_delete_export(self, path: Path) -> None:
        path = path.resolve()
        reply = QMessageBox.question(
            self,
            tr("delete_export_title"),
            tr("delete_export_body", name=path.name),
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
            QMessageBox.StandardButton.No,
        )
        if reply != QMessageBox.StandardButton.Yes:
            return
        try:
            path.unlink(missing_ok=True)
        except OSError as exc:
            QMessageBox.warning(self, tr("delete_failed_title"), str(exc))
            return
        self.append_status_message(tr("deleted_export", name=path.name), severity="success")
        self.refresh_history()

    def _apply_outcome_badge(self, item: QTableWidgetItem, outcome: str) -> None:
        font = item.font()
        font.setBold(True)
        item.setFont(font)
        if outcome == "PASS":
            item.setForeground(QBrush(QColor(token_color("status.successInk"))))
        elif outcome == "FAIL":
            item.setForeground(QBrush(QColor(token_color("status.errorInk"))))
        else:
            item.setForeground(QBrush(QColor(token_color("ink.tertiary"))))

    def _entry_for_table_row(self, row: int) -> ExportHistoryEntry | None:
        item = self.history_table.item(row, 0)
        if item is None:
            return None
        path_str = item.data(Qt.UserRole)
        if not path_str:
            return None
        path = Path(path_str)
        for entry in self._all_history_entries:
            if entry.path == path:
                return entry
        return None

    def _show_history_context_menu(self, pos) -> None:
        row = self.history_table.rowAt(pos.y())
        if row < 0:
            return
        entry = self._entry_for_table_row(row)
        if entry is None:
            return
        menu = QMenu(self)
        open_action = QAction(tr("ctx_open_file"), self)
        open_action.triggered.connect(lambda: QDesktopServices.openUrl(QUrl.fromLocalFile(str(entry.path))))
        reveal_action = QAction(tr("ctx_reveal_folder"), self)
        reveal_action.triggered.connect(lambda: self._reveal_path_in_folder(entry.path))
        copy_action = QAction(tr("ctx_copy_path"), self)
        copy_action.triggered.connect(lambda: QGuiApplication.clipboard().setText(str(entry.path)))
        preview_action = QAction(tr("ctx_preview_csv"), self)
        preview_action.triggered.connect(lambda: self._preview_csv(entry.path))
        delete_action = QAction(tr("ctx_delete_file"), self)
        delete_action.triggered.connect(lambda: self._confirm_delete_export(entry.path))
        menu.addAction(open_action)
        menu.addAction(reveal_action)
        menu.addAction(copy_action)
        menu.addSeparator()
        menu.addAction(preview_action)
        menu.addSeparator()
        menu.addAction(delete_action)
        menu.exec(self.history_table.viewport().mapToGlobal(pos))

    def _reveal_path_in_folder(self, path: Path) -> None:
        path = path.resolve()
        if sys.platform == "win32":
            subprocess.run(["explorer", "/select,", os.path.normpath(str(path))], check=False)
        else:
            QDesktopServices.openUrl(QUrl.fromLocalFile(str(path.parent)))

    def _preview_csv(self, path: Path) -> None:
        CsvPreviewDialog(path, self).exec()

    def refresh_history(self) -> None:
        self._all_history_entries = list_export_history()
        self._update_history_table_rows()
        count = len(self._all_history_entries)
        self.sidebar.set_badge("file_explorer", str(count) if count else "")
        self.exports_path_label.setText(str(get_exports_dir()))

    def open_selected_export(self, row: int, column: int) -> None:
        if column == 3:
            return
        item = self.history_table.item(row, 0)
        if item is None:
            return

        path = item.data(Qt.UserRole)
        if not path:
            return

        QDesktopServices.openUrl(QUrl.fromLocalFile(path))

    def open_exports_folder(self) -> None:
        exports_dir = ensure_exports_dir()
        QDesktopServices.openUrl(QUrl.fromLocalFile(str(exports_dir)))

    def _change_exports_folder(self) -> None:
        current = str(get_exports_dir())
        chosen = QFileDialog.getExistingDirectory(self, tr("select_exports_folder"), current)
        if not chosen:
            return
        new_dir = Path(chosen)
        set_exports_dir(new_dir)
        ensure_exports_dir()
        self.exports_path_label.setText(str(new_dir))
        self.append_status_message(tr("folder_changed", path=new_dir), severity="success")
        self.refresh_history()

    def _reset_exports_folder(self) -> None:
        reset_exports_dir()
        ensure_exports_dir()
        self.exports_path_label.setText(str(get_exports_dir()))
        self.append_status_message(tr("folder_reset"), severity="success")
        self.refresh_history()

    def _open_settings(self) -> None:
        dialog = SettingsDialog(self._app_settings, self)
        if dialog.exec() == QDialog.Accepted:
            self._app_settings = dialog.get_settings()
            save_all_settings(self._app_settings)
            set_language(self._app_settings.get("language", "English"))
            self._apply_current_settings()
            self._retranslate_ui()
            self.append_status_message(tr("settings_saved"), severity="success")

    def _apply_current_settings(self) -> None:
        if self._app_settings.get("dark_mode", False):
            self._apply_dark_styles()
        else:
            self._apply_styles()
        self._apply_font_size(self._app_settings.get("font_size", "Medium"))
        reduced = bool(self._app_settings.get("reduced_motion", False))
        if hasattr(self, "sidebar"):
            self.sidebar.set_reduced_motion(reduced)
        if hasattr(self, "log_drawer"):
            self.log_drawer.set_reduced_motion(reduced)
        if hasattr(self, "_oct_step_progress"):
            self._oct_step_progress.set_reduced_motion(reduced)

    def _apply_font_size(self, size_name: str) -> None:
        sizes = {"Small": 12, "Medium": 14, "Large": 16}
        px = sizes.get(size_name, 14)
        root = self.centralWidget()
        if root is not None:
            font = root.font()
            font.setPointSize(px)
            root.setFont(font)

    def _retranslate_ui(self) -> None:
        self.setWindowTitle(tr("app_title"))

        self._eyebrow.setText(tr("eyebrow"))
        self._hero_title.setText(tr("hero_title"))
        self._hero_subtitle.setText(tr("hero_subtitle"))

        self.sidebar.set_item_label("file_explorer", tr("tab_file_explorer"))
        self.sidebar.set_item_label("device_tools", tr("tab_device_tools"))
        self.sidebar.set_item_label("one_click_test", tr("tab_one_click_test"))
        self.sidebar.set_settings_label(tr("nav_settings"))
        self.sidebar.set_collapse_labels(tr("nav_collapse"), tr("nav_expand"))
        self.log_drawer.set_title(tr("activity_log"))
        self.log_drawer.set_labels(tr("copy"), tr("clear"))

        self.refresh_history_button.setText(tr("refresh_list"))
        self.refresh_history_button.setToolTip(tr("tooltip_refresh"))
        self.open_folder_button.setText(tr("open_exports_folder"))
        self.open_folder_button.setToolTip(tr("tooltip_open_folder"))
        self.change_folder_button.setText(tr("change_folder"))
        self.change_folder_button.setToolTip(tr("tooltip_change_folder"))
        self.reset_folder_button.setText(tr("reset_to_default"))
        self.reset_folder_button.setToolTip(tr("tooltip_reset_folder"))

        self.history_search.setPlaceholderText(tr("search_by_filename"))
        self._outcome_label.setText(tr("outcome"))
        self.clear_history_filters_button.setText(tr("clear_filters"))
        self.history_table.setHorizontalHeaderLabels(
            [tr("col_filename"), tr("col_saved_time"), tr("col_outcome"), ""]
        )

        self.empty_go_device_tools.setText(tr("go_to_device_tools"))
        self.empty_refresh.setText(tr("refresh_list_lower"))

        self._connection_heading.setText(tr("connection_readiness"))
        self._last_heading.setText(tr("last_successful_operation"))

        self._export_card_title.setText(tr("finished_board_export"))
        self._export_card_subtitle.setText(tr("export_card_subtitle"))
        self.extract_button.setText(tr("extract_data"))
        self.extract_button.setToolTip(tr("tooltip_extract"))

        self._test_fw_title.setText(tr("test_firmware_title"))
        self._test_fw_subtitle.setText(tr("test_firmware_subtitle"))
        self.upload_test_button.setText(tr("upload_test_firmware"))
        self.upload_test_button.setToolTip(tr("tooltip_upload_test"))

        self._prod_fw_title.setText(tr("production_firmware_title"))
        self._prod_fw_subtitle.setText(tr("production_firmware_subtitle"))
        self.upload_production_button.setText(tr("upload_production_firmware"))
        self.upload_production_button.setToolTip(tr("tooltip_upload_production"))

        self._oct_conn_title.setText(tr("connection_readiness"))
        self._oct_title.setText(tr("oct_title"))
        self._oct_subtitle.setText(tr("oct_subtitle"))
        self.oct_start_btn.setText(tr("oct_start_btn"))
        self.oct_start_btn.setToolTip(tr("oct_start_tooltip"))
        self._oct_step_progress.set_labels([tr(key) for key in self._oct_step_keys])

        self.refresh_history()
        self._update_last_operation_ui()
        idx = self.pages.currentIndex()
        if idx == PAGE_INDEX_DEVICE_TOOLS:
            self._refresh_connection_status()
        elif idx == PAGE_INDEX_ONE_CLICK:
            self._refresh_oct_connection()

def main() -> int:
    app = QApplication(sys.argv)
    app.setApplicationName(tr("app_title"))
    window = ExportAppWindow()
    if window.icon_path is not None:
        app.setWindowIcon(QIcon(str(window.icon_path)))
    window.showFullScreen()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
