"""Horizontal step progress rail for the One Click Test page.

Replaces the emoji-in-boxes row with a connected pill rail. Each step is drawn
with a circular indicator plus a label, connected by a thin line that fills in
as the test progresses:

    [OK]---[OK]---[>>]---[ 4 ]---[ 5 ]  ...
     Upload  Comms  DigIn  AnIn   DigOut
"""

from __future__ import annotations

from enum import Enum
from typing import Optional

from PySide6.QtCore import QRectF, QTimer, Qt
from PySide6.QtGui import (
    QBrush,
    QColor,
    QFont,
    QPainter,
    QPen,
)
from PySide6.QtWidgets import QSizePolicy, QWidget

from theme import color as token_color


class StepState(str, Enum):
    PENDING = "pending"
    ACTIVE = "active"
    DONE = "done"
    FAILED = "failed"


class StepProgressBar(QWidget):
    """Custom-painted multi-step progress indicator."""

    NODE_DIAMETER = 30
    NODE_Y_OFFSET = 14
    LABEL_MAX_LINES = 2
    CONNECTOR_HEIGHT = 3

    def __init__(self, parent: Optional[QWidget] = None) -> None:
        super().__init__(parent)
        self.setObjectName("stepProgressBar")
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
        self.setMinimumHeight(88)

        self._labels: list[str] = []
        self._states: list[StepState] = []
        self._pulse_phase: float = 0.0
        self._reduced_motion: bool = False

        self._pulse_timer = QTimer(self)
        self._pulse_timer.setInterval(60)
        self._pulse_timer.timeout.connect(self._on_pulse_tick)

    # ------------------------------------------------------------------
    #  Public API
    # ------------------------------------------------------------------
    def set_steps(self, labels: list[str]) -> None:
        self._labels = list(labels)
        self._states = [StepState.PENDING for _ in labels]
        if self._labels:
            self._states[0] = StepState.ACTIVE
        self._maybe_toggle_pulse()
        self.update()

    def set_labels(self, labels: list[str]) -> None:
        if len(labels) != len(self._labels):
            self._labels = list(labels)
            if len(self._states) != len(self._labels):
                self._states = [StepState.PENDING for _ in self._labels]
        else:
            self._labels = list(labels)
        self.update()

    def set_reduced_motion(self, enabled: bool) -> None:
        self._reduced_motion = enabled
        self._maybe_toggle_pulse()

    def reset(self) -> None:
        self._states = [StepState.PENDING for _ in self._labels]
        if self._labels:
            self._states[0] = StepState.ACTIVE
        self._maybe_toggle_pulse()
        self.update()

    def set_active(self, index: int) -> None:
        """Mark everything before ``index`` as done and ``index`` as active."""
        for i in range(len(self._states)):
            if i < index:
                self._states[i] = StepState.DONE
            elif i == index:
                self._states[i] = StepState.ACTIVE
            else:
                self._states[i] = StepState.PENDING
        self._maybe_toggle_pulse()
        self.update()

    def mark_all_done(self) -> None:
        self._states = [StepState.DONE for _ in self._labels]
        self._maybe_toggle_pulse()
        self.update()

    def set_state(self, index: int, state: StepState) -> None:
        if 0 <= index < len(self._states):
            self._states[index] = state
            self._maybe_toggle_pulse()
            self.update()

    def mark_failed(self, index: int) -> None:
        self.set_state(index, StepState.FAILED)

    def state_at(self, index: int) -> StepState:
        if 0 <= index < len(self._states):
            return self._states[index]
        return StepState.PENDING

    # ------------------------------------------------------------------
    #  Animation
    # ------------------------------------------------------------------
    def _maybe_toggle_pulse(self) -> None:
        has_active = any(s == StepState.ACTIVE for s in self._states)
        if has_active and not self._reduced_motion:
            if not self._pulse_timer.isActive():
                self._pulse_timer.start()
        else:
            if self._pulse_timer.isActive():
                self._pulse_timer.stop()
            self._pulse_phase = 0.0
            self.update()

    def _on_pulse_tick(self) -> None:
        self._pulse_phase = (self._pulse_phase + 0.08) % 1.0
        self.update()

    # ------------------------------------------------------------------
    #  Painting
    # ------------------------------------------------------------------
    def paintEvent(self, event) -> None:  # noqa: ANN001 - Qt event
        if not self._labels:
            return
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing, True)

        c_pending = QColor(token_color("border.strong"))
        c_pending_fill = QColor(token_color("surface.card"))
        c_pending_ink = QColor(token_color("ink.tertiary"))

        c_active = QColor(token_color("brand.red"))
        c_active_ink = QColor("#ffffff")
        c_active_halo = QColor(token_color("brand.red"))

        c_done = QColor(token_color("status.successInk"))
        c_done_fill = QColor(token_color("status.successBg"))
        c_done_mark = QColor(token_color("status.successInk"))

        c_failed = QColor(token_color("status.errorInk"))
        c_failed_fill = QColor(token_color("status.errorBg"))

        c_connector_off = QColor(token_color("border.default"))
        c_connector_on = QColor(token_color("brand.redAccent"))

        c_label_ink = QColor(token_color("ink.secondary"))
        c_label_active = QColor(token_color("ink.primary"))

        count = len(self._labels)
        width = self.width()
        inner_pad = 8
        usable = max(width - inner_pad * 2, 10)
        step_w = usable / count

        centers = [inner_pad + step_w * (i + 0.5) for i in range(count)]
        node_r = self.NODE_DIAMETER / 2
        node_y = self.NODE_Y_OFFSET + node_r

        connector_y = node_y - self.CONNECTOR_HEIGHT / 2
        for i in range(count - 1):
            x1 = centers[i] + node_r
            x2 = centers[i + 1] - node_r
            if x2 <= x1:
                continue
            rect = QRectF(x1, connector_y, x2 - x1, self.CONNECTOR_HEIGHT)
            left_done = self._states[i] in (StepState.DONE, StepState.FAILED) or (
                self._states[i] == StepState.ACTIVE and False
            )
            right_done = self._states[i + 1] in (StepState.DONE, StepState.FAILED)
            if left_done:
                painter.setBrush(QBrush(c_connector_on))
            else:
                painter.setBrush(QBrush(c_connector_off))
            painter.setPen(Qt.NoPen)
            painter.drawRoundedRect(rect, 1.5, 1.5)
            if left_done and not right_done and self._states[i + 1] == StepState.ACTIVE:
                painter.setBrush(QBrush(c_connector_on))
                painter.drawRoundedRect(rect, 1.5, 1.5)

        for i, label in enumerate(self._labels):
            state = self._states[i]
            cx = centers[i]
            node_rect = QRectF(cx - node_r, node_y - node_r, node_r * 2, node_r * 2)

            if state == StepState.PENDING:
                painter.setBrush(QBrush(c_pending_fill))
                painter.setPen(QPen(c_pending, 2))
                painter.drawEllipse(node_rect)
                painter.setPen(QPen(c_pending_ink))
                f = QFont(self.font())
                f.setPointSize(9)
                f.setBold(True)
                painter.setFont(f)
                painter.drawText(node_rect, Qt.AlignCenter, str(i + 1))

            elif state == StepState.ACTIVE:
                if not self._reduced_motion:
                    halo = QColor(c_active_halo)
                    halo.setAlphaF(0.15 + 0.15 * abs(0.5 - self._pulse_phase) * 2)
                    halo_r = node_r + 4 + 2 * (0.5 - abs(0.5 - self._pulse_phase)) * 2
                    halo_rect = QRectF(
                        cx - halo_r, node_y - halo_r, halo_r * 2, halo_r * 2
                    )
                    painter.setBrush(QBrush(halo))
                    painter.setPen(Qt.NoPen)
                    painter.drawEllipse(halo_rect)

                painter.setBrush(QBrush(c_active))
                painter.setPen(Qt.NoPen)
                painter.drawEllipse(node_rect)
                painter.setPen(QPen(c_active_ink))
                f = QFont(self.font())
                f.setPointSize(11)
                f.setBold(True)
                painter.setFont(f)
                painter.drawText(node_rect, Qt.AlignCenter, "\u25B6")

            elif state == StepState.DONE:
                painter.setBrush(QBrush(c_done_fill))
                painter.setPen(QPen(c_done, 2))
                painter.drawEllipse(node_rect)
                painter.setPen(QPen(c_done_mark, 2.4))
                cx_ = node_rect.center().x()
                cy_ = node_rect.center().y()
                painter.drawLine(
                    int(cx_ - 6), int(cy_ + 0.5),
                    int(cx_ - 1), int(cy_ + 5),
                )
                painter.drawLine(
                    int(cx_ - 1), int(cy_ + 5),
                    int(cx_ + 7), int(cy_ - 4),
                )

            elif state == StepState.FAILED:
                painter.setBrush(QBrush(c_failed_fill))
                painter.setPen(QPen(c_failed, 2))
                painter.drawEllipse(node_rect)
                painter.setPen(QPen(c_failed, 2.4))
                cx_ = node_rect.center().x()
                cy_ = node_rect.center().y()
                painter.drawLine(int(cx_ - 5), int(cy_ - 5), int(cx_ + 5), int(cy_ + 5))
                painter.drawLine(int(cx_ - 5), int(cy_ + 5), int(cx_ + 5), int(cy_ - 5))

            f = QFont(self.font())
            f.setPointSize(9)
            f.setBold(state == StepState.ACTIVE)
            painter.setFont(f)
            if state == StepState.ACTIVE:
                painter.setPen(QPen(c_label_active))
            elif state == StepState.DONE:
                painter.setPen(QPen(c_done))
            elif state == StepState.FAILED:
                painter.setPen(QPen(c_failed))
            else:
                painter.setPen(QPen(c_label_ink))

            label_rect = QRectF(
                cx - step_w / 2 + 2,
                node_y + node_r + 8,
                step_w - 4,
                self.height() - node_y - node_r - 10,
            )
            painter.drawText(
                label_rect,
                int(Qt.AlignHCenter | Qt.AlignTop | Qt.TextWordWrap),
                label,
            )

        painter.end()
