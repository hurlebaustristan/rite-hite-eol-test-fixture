"""Bottom-docked collapsible activity log drawer.

Replaces the right-side status rail. The drawer has a 40px header with a title,
severity pill counts, Copy and Clear actions, and a chevron toggle. Expanding
reveals a monospace text area where messages are rendered with severity
coloring and a dimmed timestamp prefix.
"""

from __future__ import annotations

from datetime import datetime
from typing import Optional

from PySide6.QtCore import (
    QEasingCurve,
    QPropertyAnimation,
    Signal,
    Qt,
)
from PySide6.QtGui import QGuiApplication, QTextCursor
from PySide6.QtWidgets import (
    QFrame,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QSizePolicy,
    QTextEdit,
    QVBoxLayout,
    QWidget,
)

from theme import TOKENS, color, qta_icon


_SEVERITY_INK = {
    "info": color("ink.primary"),
    "success": color("status.successInk"),
    "error": color("status.errorInk"),
    "warn": color("status.warnInk"),
}


class LogDrawer(QFrame):
    """IDE-style log panel docked at the bottom of the window."""

    expandedChanged = Signal(bool)

    def __init__(
        self,
        *,
        title: str = "Activity Log",
        copy_label: str = "Copy",
        clear_label: str = "Clear",
        reduced_motion: bool = False,
        parent: Optional[QWidget] = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("logDrawer")
        self._title_text = title
        self._copy_label = copy_label
        self._clear_label = clear_label
        self._expanded: bool = False
        self._reduced_motion = reduced_motion
        self._counts = {"info": 0, "success": 0, "error": 0}

        self._collapsed_h = int(TOKENS["drawer"]["collapsedHeight"])  # type: ignore[index]
        self._expanded_h = int(TOKENS["drawer"]["expandedHeight"])  # type: ignore[index]
        self._target_h = self._collapsed_h

        self.setFixedHeight(self._collapsed_h)
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)

        root = QVBoxLayout(self)
        root.setContentsMargins(0, 0, 0, 0)
        root.setSpacing(0)

        self._header = self._build_header()
        root.addWidget(self._header)

        self._body = QTextEdit()
        self._body.setObjectName("statusPanel")
        self._body.setReadOnly(True)
        self._body.setVisible(False)
        root.addWidget(self._body, 1)

        self._anim = QPropertyAnimation(self, b"maximumHeight", self)
        self._anim.setDuration(int(TOKENS["motion"]["fast"]))  # type: ignore[index]
        self._anim.setEasingCurve(QEasingCurve.OutCubic)
        self._anim_min = QPropertyAnimation(self, b"minimumHeight", self)
        self._anim_min.setDuration(int(TOKENS["motion"]["fast"]))  # type: ignore[index]
        self._anim_min.setEasingCurve(QEasingCurve.OutCubic)
        self._anim.finished.connect(self._finalize_animation)

    def _build_header(self) -> QWidget:
        header = QWidget()
        header.setObjectName("logDrawerHeader")
        header.setFixedHeight(self._collapsed_h)
        header.setCursor(Qt.PointingHandCursor)
        row = QHBoxLayout(header)
        row.setContentsMargins(14, 0, 10, 0)
        row.setSpacing(10)

        self._chevron_btn = QPushButton()
        self._chevron_btn.setObjectName("drawerToggle")
        self._chevron_btn.setCursor(Qt.PointingHandCursor)
        self._chevron_btn.setIcon(qta_icon("fa6s.chevron-up", "ink.tertiary"))
        self._chevron_btn.setToolTip("Show activity log")
        self._chevron_btn.clicked.connect(self.toggle)
        row.addWidget(self._chevron_btn)

        self._title_lbl = QLabel(self._title_text)
        self._title_lbl.setObjectName("logDrawerTitle")
        row.addWidget(self._title_lbl)

        self._info_pill = self._make_pill("info", "0")
        self._success_pill = self._make_pill("success", "0")
        self._error_pill = self._make_pill("error", "0")
        row.addSpacing(4)
        row.addWidget(self._info_pill)
        row.addWidget(self._success_pill)
        row.addWidget(self._error_pill)

        row.addStretch(1)

        self._copy_btn = QPushButton(self._copy_label)
        self._copy_btn.setObjectName("statusMiniButton")
        self._copy_btn.setIcon(qta_icon("fa6s.copy", "ink.secondary"))
        self._copy_btn.setToolTip("Copy log to clipboard")
        self._copy_btn.clicked.connect(self._copy_log)
        row.addWidget(self._copy_btn)

        self._clear_btn = QPushButton(self._clear_label)
        self._clear_btn.setObjectName("statusMiniButton")
        self._clear_btn.setIcon(qta_icon("fa6s.broom", "ink.secondary"))
        self._clear_btn.setToolTip("Clear the activity log")
        self._clear_btn.clicked.connect(self.clear)
        row.addWidget(self._clear_btn)

        header.mousePressEvent = self._on_header_clicked  # type: ignore[assignment]
        return header

    def _make_pill(self, severity: str, text: str) -> QLabel:
        lbl = QLabel(text)
        lbl.setObjectName("logCount")
        lbl.setProperty("severity", severity)
        lbl.setAlignment(Qt.AlignCenter)
        lbl.setMinimumWidth(28)
        return lbl

    def _on_header_clicked(self, event) -> None:  # noqa: ANN001 - Qt event
        if event.button() == Qt.LeftButton:
            self.toggle()
        event.accept()

    # ------------------------------------------------------------------
    #  Public API
    # ------------------------------------------------------------------
    def set_reduced_motion(self, enabled: bool) -> None:
        self._reduced_motion = enabled

    def set_title(self, text: str) -> None:
        self._title_text = text
        self._title_lbl.setText(text)

    def set_labels(self, copy_label: str, clear_label: str) -> None:
        self._copy_label = copy_label
        self._clear_label = clear_label
        self._copy_btn.setText(copy_label)
        self._clear_btn.setText(clear_label)

    def toggle(self) -> None:
        self.set_expanded(not self._expanded)

    def is_expanded(self) -> bool:
        return self._expanded

    def set_expanded(self, expanded: bool) -> None:
        if expanded == self._expanded:
            return
        self._expanded = expanded

        target = self._target_height(expanded)
        start = self.height()
        self._target_h = target

        self._chevron_btn.setIcon(
            qta_icon(
                "fa6s.chevron-down" if expanded else "fa6s.chevron-up",
                "ink.tertiary",
            )
        )
        self._chevron_btn.setToolTip("Hide activity log" if expanded else "Show activity log")
        self._body.setVisible(expanded)

        if self._reduced_motion:
            self.setFixedHeight(target)
        else:
            self._anim.stop()
            self._anim_min.stop()
            self.setMinimumHeight(0)
            self.setMaximumHeight(16777215)
            self._anim.setStartValue(start)
            self._anim.setEndValue(target)
            self._anim_min.setStartValue(start)
            self._anim_min.setEndValue(target)
            self._anim.start()
            self._anim_min.start()

        self.expandedChanged.emit(expanded)

    def _target_height(self, expanded: bool) -> int:
        if not expanded:
            return self._collapsed_h
        window = self.window()
        base_height = window.height() if window is not None else self._expanded_h
        dynamic_cap = max(self._collapsed_h + 80, int(base_height * 0.28))
        return min(self._expanded_h, dynamic_cap)

    def _finalize_animation(self) -> None:
        self.setFixedHeight(self._target_h)

    def append(self, message: str, severity: str = "info") -> None:
        ts = datetime.now().strftime("%H:%M:%S")
        ink = _SEVERITY_INK.get(severity, _SEVERITY_INK["info"])
        icon_char = {
            "success": "[OK]",
            "error": "[ERR]",
            "warn": "[WARN]",
            "info": "",
        }.get(severity, "")
        icon_part = (
            f'<span style="color:{ink}; font-weight:600">{icon_char}</span> '
            if icon_char
            else ""
        )
        html = (
            f'<span style="color:{color("ink.muted")}">[{ts}]</span> '
            f"{icon_part}"
            f'<span style="color:{ink}">{message}</span>'
        )
        self._body.append(html)
        self._body.moveCursor(QTextCursor.MoveOperation.End)
        self._bump_count(severity)

    def set_message(self, message: str, severity: str = "info") -> None:
        self._body.clear()
        self._counts = {"info": 0, "success": 0, "error": 0}
        self._refresh_counts()
        self.append(message, severity)

    def clear(self) -> None:
        self._body.clear()
        self._counts = {"info": 0, "success": 0, "error": 0}
        self._refresh_counts()

    def _bump_count(self, severity: str) -> None:
        bucket = "error" if severity == "error" else (
            "success" if severity == "success" else "info"
        )
        self._counts[bucket] += 1
        self._refresh_counts()

    def _refresh_counts(self) -> None:
        self._info_pill.setText(str(self._counts["info"]))
        self._success_pill.setText(str(self._counts["success"]))
        self._error_pill.setText(str(self._counts["error"]))
        for pill in (self._info_pill, self._success_pill, self._error_pill):
            pill.style().unpolish(pill)
            pill.style().polish(pill)

    def _copy_log(self) -> None:
        QGuiApplication.clipboard().setText(self._body.toPlainText())

    def plain_text(self) -> str:
        return self._body.toPlainText()
