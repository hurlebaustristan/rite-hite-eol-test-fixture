"""Collapsible left-rail navigation for the EOL Export Software.

The sidebar hosts the three main pages (File Explorer, Device Tools, One Click
Test) plus a footer with Settings and optional collapse toggle. It emits
``pageChanged(int)`` so the main window can drive a QStackedWidget, and
``settingsRequested()`` / ``collapsedChanged(bool)`` for the host window to
react.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Callable, Optional

from PySide6.QtCore import (
    QEasingCurve,
    QPropertyAnimation,
    Qt,
    Signal,
)
from PySide6.QtGui import QIcon
from PySide6.QtWidgets import (
    QFrame,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QSizePolicy,
    QSpacerItem,
    QVBoxLayout,
    QWidget,
)

from theme import TOKENS, qta_icon


@dataclass
class SidebarItem:
    """Lightweight descriptor for a sidebar navigation entry."""

    key: str
    label: str
    icon_name: str
    tooltip: str = ""


class _NavButton(QPushButton):
    """Internal QPushButton with an optional numeric badge on the right."""

    def __init__(self, item: SidebarItem, parent: Optional[QWidget] = None) -> None:
        super().__init__(parent)
        self._item = item
        self.setObjectName("sidebarItem")
        self.setCheckable(True)
        self.setAutoExclusive(False)
        self.setCursor(Qt.PointingHandCursor)
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
        self.setFixedHeight(int(TOKENS["sidebar"]["itemHeight"]))  # type: ignore[index]
        self.setIconSize(self.iconSize() * 1.15)
        self.setProperty("active", False)
        self.setToolTip(item.tooltip or item.label)

        self._label_full = item.label

        layout = QHBoxLayout(self)
        layout.setContentsMargins(14, 0, 10, 0)
        layout.setSpacing(12)

        self._icon_label = QLabel()
        self._icon_label.setFixedSize(22, 22)
        self._icon_label.setAlignment(Qt.AlignCenter)
        self._icon_label.setAttribute(Qt.WA_TransparentForMouseEvents, True)
        layout.addWidget(self._icon_label)

        self._text_label = QLabel(item.label)
        self._text_label.setAttribute(Qt.WA_TransparentForMouseEvents, True)
        self._text_label.setStyleSheet(
            f"background: transparent; color: {TOKENS['ink']['secondary']};"  # type: ignore[index]
            " font-weight: 600;"
        )
        layout.addWidget(self._text_label, 1)

        self._badge = QLabel()
        self._badge.setObjectName("sidebarBadge")
        self._badge.setAlignment(Qt.AlignCenter)
        self._badge.setAttribute(Qt.WA_TransparentForMouseEvents, True)
        self._badge.setVisible(False)
        layout.addWidget(self._badge, 0)

        self._apply_icon_color("ink.secondary")

    @property
    def key(self) -> str:
        return self._item.key

    def _apply_icon_color(self, token: str) -> None:
        icon = qta_icon(self._item.icon_name, token)
        pix = icon.pixmap(20, 20)
        if not pix.isNull():
            self._icon_label.setPixmap(pix)

    def set_active(self, active: bool) -> None:
        self.setProperty("active", active)
        self._apply_icon_color("brand.red" if active else "ink.secondary")
        if active:
            self._text_label.setStyleSheet(
                f"background: transparent; color: {TOKENS['brand']['red']};"  # type: ignore[index]
                " font-weight: 700;"
            )
        else:
            self._text_label.setStyleSheet(
                f"background: transparent; color: {TOKENS['ink']['secondary']};"  # type: ignore[index]
                " font-weight: 600;"
            )
        self.style().unpolish(self)
        self.style().polish(self)

    def set_collapsed(self, collapsed: bool) -> None:
        self._text_label.setVisible(not collapsed)
        self._badge.setVisible((not collapsed) and bool(self._badge.text()))
        if collapsed:
            self.setToolTip(self._item.tooltip or self._item.label)
            self.layout().setContentsMargins(0, 0, 0, 0)
            self.layout().setAlignment(Qt.AlignCenter)
        else:
            self.layout().setAlignment(Qt.AlignLeft | Qt.AlignVCenter)
            self.layout().setContentsMargins(14, 0, 10, 0)

    def set_badge(self, text: str) -> None:
        if text:
            self._badge.setText(text)
            self._badge.setVisible(True)
        else:
            self._badge.clear()
            self._badge.setVisible(False)

    def set_label(self, label: str) -> None:
        self._item.label = label
        self._label_full = label
        self._text_label.setText(label)


class Sidebar(QFrame):
    """Vertical navigation rail; collapses to icons-only on demand."""

    pageChanged = Signal(int)
    settingsRequested = Signal()
    collapsedChanged = Signal(bool)

    def __init__(
        self,
        items: list[SidebarItem],
        *,
        settings_label: str = "Settings",
        collapse_label: str = "Collapse",
        expand_label: str = "Expand",
        reduced_motion: bool = False,
        parent: Optional[QWidget] = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("sidebar")
        self._items = list(items)
        self._buttons: list[_NavButton] = []
        self._active_index: int = 0
        self._collapsed: bool = False
        self._reduced_motion = reduced_motion
        self._settings_label = settings_label
        self._collapse_label = collapse_label
        self._expand_label = expand_label

        expanded_w = int(TOKENS["sidebar"]["expandedWidth"])  # type: ignore[index]
        self.setFixedWidth(expanded_w)

        root = QVBoxLayout(self)
        root.setContentsMargins(10, 14, 10, 12)
        root.setSpacing(6)

        self._brand_row = QLabel("RITE-HITE")
        self._brand_row.setObjectName("sidebarBrand")
        self._brand_row.setAlignment(Qt.AlignLeft | Qt.AlignVCenter)
        root.addWidget(self._brand_row)

        divider_top = QWidget()
        divider_top.setObjectName("sidebarDivider")
        divider_top.setFixedHeight(1)
        root.addWidget(divider_top)

        root.addSpacing(6)

        for index, descriptor in enumerate(self._items):
            btn = _NavButton(descriptor, self)
            btn.clicked.connect(lambda _checked=False, i=index: self.set_active(i))
            self._buttons.append(btn)
            root.addWidget(btn)

        root.addItem(QSpacerItem(0, 0, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding))

        divider_bot = QWidget()
        divider_bot.setObjectName("sidebarDivider")
        divider_bot.setFixedHeight(1)
        root.addWidget(divider_bot)

        self._settings_btn = _NavButton(
            SidebarItem(
                key="_settings",
                label=settings_label,
                icon_name="fa6s.gear",
                tooltip=settings_label,
            ),
            self,
        )
        self._settings_btn.setCheckable(False)
        self._settings_btn.clicked.connect(self.settingsRequested.emit)
        root.addWidget(self._settings_btn)

        self._collapse_btn = QPushButton(collapse_label)
        self._collapse_btn.setObjectName("sidebarCollapseToggle")
        self._collapse_btn.setCursor(Qt.PointingHandCursor)
        self._collapse_btn.clicked.connect(self.toggle_collapsed)
        root.addWidget(self._collapse_btn)

        self._anim = QPropertyAnimation(self, b"maximumWidth", self)
        self._anim.setDuration(int(TOKENS["motion"]["normal"]))  # type: ignore[index]
        self._anim.setEasingCurve(QEasingCurve.OutCubic)
        self._anim_min = QPropertyAnimation(self, b"minimumWidth", self)
        self._anim_min.setDuration(int(TOKENS["motion"]["normal"]))  # type: ignore[index]
        self._anim_min.setEasingCurve(QEasingCurve.OutCubic)

        if self._buttons:
            self.set_active(0)

        self._update_collapse_icon()

    # ------------------------------------------------------------------
    #  Public API
    # ------------------------------------------------------------------
    def set_active(self, index: int) -> None:
        if index < 0 or index >= len(self._buttons):
            return
        self._active_index = index
        for i, btn in enumerate(self._buttons):
            btn.set_active(i == index)
            btn.setChecked(i == index)
        self.pageChanged.emit(index)

    def active_index(self) -> int:
        return self._active_index

    def set_badge(self, key: str, text: str) -> None:
        for btn in self._buttons:
            if btn.key == key:
                btn.set_badge(text)
                return

    def set_item_label(self, key: str, label: str) -> None:
        for btn in self._buttons:
            if btn.key == key:
                btn.set_label(label)
                return

    def set_settings_label(self, label: str) -> None:
        self._settings_label = label
        self._settings_btn.set_label(label)

    def set_collapse_labels(self, collapse_label: str, expand_label: str) -> None:
        self._collapse_label = collapse_label
        self._expand_label = expand_label
        self._update_collapse_icon()

    def toggle_collapsed(self) -> None:
        self.set_collapsed(not self._collapsed)

    def is_collapsed(self) -> bool:
        return self._collapsed

    def set_reduced_motion(self, enabled: bool) -> None:
        self._reduced_motion = enabled

    def set_collapsed(self, collapsed: bool) -> None:
        if collapsed == self._collapsed:
            return
        self._collapsed = collapsed

        start_w = self.width()
        target_w = int(
            TOKENS["sidebar"]["collapsedWidth" if collapsed else "expandedWidth"]
        )  # type: ignore[index]

        for btn in self._buttons:
            btn.set_collapsed(collapsed)
        self._settings_btn.set_collapsed(collapsed)
        self._brand_row.setVisible(not collapsed)
        self._update_collapse_icon()

        if self._reduced_motion:
            self.setFixedWidth(target_w)
        else:
            self._anim.stop()
            self._anim_min.stop()
            self._anim.setStartValue(start_w)
            self._anim.setEndValue(target_w)
            self._anim_min.setStartValue(start_w)
            self._anim_min.setEndValue(target_w)
            self.setMinimumWidth(0)
            self.setMaximumWidth(16777215)
            self._anim.start()
            self._anim_min.start()

            def _finalize() -> None:
                self.setFixedWidth(target_w)

            self._anim.finished.connect(_finalize, Qt.UniqueConnection)

        self.collapsedChanged.emit(collapsed)

    def _update_collapse_icon(self) -> None:
        icon_name = "fa6s.angles-right" if self._collapsed else "fa6s.angles-left"
        self._collapse_btn.setIcon(qta_icon(icon_name, "ink.tertiary"))
        if self._collapsed:
            self._collapse_btn.setText("")
            self._collapse_btn.setToolTip(self._expand_label)
        else:
            self._collapse_btn.setText(self._collapse_label)
            self._collapse_btn.setToolTip(self._collapse_label)
