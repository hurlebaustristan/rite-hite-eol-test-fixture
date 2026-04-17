"""Centralized design tokens and stylesheet factory for the EOL Export Software.

All visual design decisions (color, spacing, radii, shadows, typography) live here
so the rest of the application can stay focused on behavior. Widgets should pull
values via ``TOKENS[...]`` and apply them using the helpers at the bottom of this
module instead of hard-coding hex values inline.
"""

from __future__ import annotations

from typing import Optional

from PySide6.QtGui import QColor
from PySide6.QtWidgets import QGraphicsDropShadowEffect, QWidget


# ---------------------------------------------------------------------------
#  Design tokens
# ---------------------------------------------------------------------------

TOKENS: dict[str, object] = {
    "brand": {
        "red": "#c8102e",
        "redHover": "#a50d25",
        "redPressed": "#85091d",
        "redAccent": "#e0194a",
        "redTint": "#fdecef",
        "redTintStrong": "#f8d7dd",
    },
    "surface": {
        "app": "#f4f5f7",
        "card": "#ffffff",
        "subtle": "#fafafa",
        "inset": "#f0f1f4",
        "raised": "#ffffff",
        "overlay": "rgba(15, 23, 42, 0.45)",
    },
    "ink": {
        "primary": "#0f172a",
        "secondary": "#475569",
        "tertiary": "#64748b",
        "muted": "#94a3b8",
        "inverse": "#ffffff",
    },
    "border": {
        "default": "#e2e4e8",
        "strong": "#cfd3da",
        "subtle": "#ebedf0",
        "focus": "#c8102e",
    },
    "status": {
        "successBg": "#e7f5ec",
        "successInk": "#166534",
        "successBorder": "#bfe3ce",
        "warnBg": "#fff7e6",
        "warnInk": "#b45309",
        "warnBorder": "#f5d7a1",
        "errorBg": "#fdecef",
        "errorInk": "#b91c1c",
        "errorBorder": "#f4bac4",
        "infoBg": "#eaf2fb",
        "infoInk": "#1e40af",
        "infoBorder": "#c9ddf5",
        "neutralBg": "#f0f1f4",
        "neutralInk": "#475569",
        "neutralBorder": "#d9dde2",
    },
    "accent": {
        "testBlueBg": "#eaf2fb",
        "testBlueInk": "#1d4ed8",
        "testBlueBorder": "#c9ddf5",
        "prodGreenBg": "#e7f5ec",
        "prodGreenInk": "#15803d",
        "prodGreenBorder": "#bfe3ce",
    },
    "radius": {
        "sm": 6,
        "md": 8,
        "lg": 12,
        "pill": 999,
    },
    "space": {
        "xs": 4,
        "sm": 8,
        "md": 12,
        "lg": 16,
        "xl": 20,
        "2xl": 24,
        "3xl": 32,
    },
    "font": {
        "family": '"Segoe UI", "Inter", Arial, sans-serif',
        "mono": '"Cascadia Mono", Consolas, "Courier New", monospace',
    },
    "shadow": {
        "sm": {"blur": 8, "dx": 0, "dy": 1, "alpha": 18},
        "md": {"blur": 16, "dx": 0, "dy": 4, "alpha": 30},
        "lg": {"blur": 28, "dx": 0, "dy": 10, "alpha": 46},
    },
    "motion": {
        "fast": 120,
        "normal": 180,
        "slow": 280,
    },
    "sidebar": {
        "collapsedWidth": 64,
        "expandedWidth": 224,
        "itemHeight": 44,
    },
    "drawer": {
        "collapsedHeight": 40,
        "expandedHeight": 220,
    },
}


def color(path: str) -> str:
    """Resolve a dotted token path to its color string."""
    node: object = TOKENS
    for segment in path.split("."):
        if not isinstance(node, dict):
            raise KeyError(f"Token path '{path}' is invalid (segment '{segment}')")
        node = node[segment]
    if not isinstance(node, str):
        raise KeyError(f"Token path '{path}' does not resolve to a color string")
    return node


# ---------------------------------------------------------------------------
#  Shadows
# ---------------------------------------------------------------------------


def apply_shadow(widget: QWidget, level: str = "md") -> QGraphicsDropShadowEffect:
    """Attach a configured drop-shadow effect to ``widget`` and return it."""
    spec = TOKENS["shadow"][level]  # type: ignore[index]
    shadow = QGraphicsDropShadowEffect(widget)
    shadow.setBlurRadius(int(spec["blur"]))
    shadow.setOffset(int(spec["dx"]), int(spec["dy"]))
    shadow.setColor(QColor(15, 23, 42, int(spec["alpha"])))
    widget.setGraphicsEffect(shadow)
    return shadow


def tune_shadow(widget: QWidget, level: str) -> None:
    """Reconfigure an existing drop-shadow to a different token level."""
    effect = widget.graphicsEffect()
    if not isinstance(effect, QGraphicsDropShadowEffect):
        apply_shadow(widget, level)
        return
    spec = TOKENS["shadow"][level]  # type: ignore[index]
    effect.setBlurRadius(int(spec["blur"]))
    effect.setOffset(int(spec["dx"]), int(spec["dy"]))
    effect.setColor(QColor(15, 23, 42, int(spec["alpha"])))


# ---------------------------------------------------------------------------
#  Icons (qtawesome wrapper)
# ---------------------------------------------------------------------------


def qta_icon(name: str, color_token: str = "ink.secondary", size_px: Optional[int] = None):
    """Return a QIcon from qtawesome, colored with a design-token color.

    Falls back to a blank QIcon if qtawesome is not installed so the rest of the
    app keeps working during dev. ``size_px`` is accepted for API symmetry; Qt
    scales icons via the widget's ``setIconSize`` in practice.
    """
    try:
        import qtawesome as qta  # type: ignore
    except ImportError:
        from PySide6.QtGui import QIcon

        return QIcon()
    return qta.icon(name, color=color(color_token))


# ---------------------------------------------------------------------------
#  Status-pill helper
# ---------------------------------------------------------------------------


_PILL_STYLES = {
    "success": ("status.successBg", "status.successInk", "status.successBorder"),
    "warn": ("status.warnBg", "status.warnInk", "status.warnBorder"),
    "error": ("status.errorBg", "status.errorInk", "status.errorBorder"),
    "info": ("status.infoBg", "status.infoInk", "status.infoBorder"),
    "neutral": ("status.neutralBg", "status.neutralInk", "status.neutralBorder"),
}


def set_status_pill(label, kind: str) -> None:
    """Apply a compact pill style to a QLabel for connection/outcome status."""
    bg_token, ink_token, border_token = _PILL_STYLES.get(kind, _PILL_STYLES["neutral"])
    label.setStyleSheet(
        f"background: {color(bg_token)};"
        f" color: {color(ink_token)};"
        f" border: 1px solid {color(border_token)};"
        f" border-radius: 6px;"
        f" padding: 6px 12px;"
        f" font-size: 13px;"
        f" font-weight: 600;"
    )


# ---------------------------------------------------------------------------
#  Light-theme stylesheet
# ---------------------------------------------------------------------------


def build_stylesheet() -> str:
    """Return the full QSS for the application chrome in the light theme."""
    c = color
    return f"""
* {{
    font-family: {TOKENS["font"]["family"]};
}}

QWidget#appRoot {{
    background: {c("surface.app")};
    color: {c("ink.primary")};
    font-size: 14px;
}}

/* ------- Masthead ------- */
QWidget#masthead {{
    background: {c("surface.card")};
    border: none;
    border-radius: 12px;
}}
QWidget#mastheadContent {{ background: transparent; }}
QLabel#brandLogo {{ background: transparent; }}
QLabel#brandFallback {{
    color: {c("brand.red")};
    font-size: 28px;
    font-weight: 900;
    font-style: italic;
    letter-spacing: -0.5px;
}}
QFrame#mastheadDivider {{
    background: {c("border.default")};
    border: none;
}}
QLabel#eyebrowLabel {{
    color: {c("brand.red")};
    font-size: 10px;
    font-weight: 700;
    letter-spacing: 1.4px;
    text-transform: uppercase;
}}
QLabel#heroTitle {{
    color: {c("ink.primary")};
    font-size: 22px;
    font-weight: 700;
}}
QLabel#heroSubtitle {{
    color: {c("ink.tertiary")};
    font-size: 13px;
    max-width: 880px;
}}
QFrame#accentLine {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 {c("brand.red")}, stop:1 {c("brand.redAccent")});
    border: none;
    border-bottom-left-radius: 12px;
    border-bottom-right-radius: 12px;
}}

/* ------- Sidebar ------- */
QFrame#sidebar {{
    background: {c("surface.card")};
    border: none;
    border-radius: 12px;
}}
QLabel#sidebarBrand {{
    color: {c("brand.red")};
    font-size: 11px;
    font-weight: 800;
    letter-spacing: 1.6px;
    padding: 6px 14px 0 14px;
}}
QWidget#sidebarDivider {{
    background: {c("border.subtle")};
    max-height: 1px;
    min-height: 1px;
}}
QPushButton#sidebarItem {{
    text-align: left;
    padding: 10px 16px 10px 14px;
    min-height: 40px;
    border: none;
    border-radius: 8px;
    color: {c("ink.secondary")};
    background: transparent;
    font-size: 13px;
    font-weight: 600;
}}
QPushButton#sidebarItem:hover {{
    background: {c("surface.inset")};
    color: {c("ink.primary")};
}}
QPushButton#sidebarItem[active="true"] {{
    background: {c("brand.redTint")};
    color: {c("brand.red")};
}}
QPushButton#sidebarCollapseToggle {{
    text-align: left;
    padding: 8px 14px;
    min-height: 36px;
    border: none;
    border-radius: 8px;
    color: {c("ink.tertiary")};
    background: transparent;
    font-size: 12px;
    font-weight: 600;
}}
QPushButton#sidebarCollapseToggle:hover {{
    background: {c("surface.inset")};
    color: {c("ink.primary")};
}}
QLabel#sidebarBadge {{
    background: {c("brand.redTint")};
    color: {c("brand.red")};
    border-radius: 9px;
    padding: 1px 7px;
    font-size: 10px;
    font-weight: 800;
    min-width: 16px;
}}

/* ------- Pages ------- */
QScrollArea#pageScrollArea, QWidget#pageScrollArea {{ background: transparent; border: none; }}
QWidget#pageRoot {{ background: transparent; }}

/* ------- Cards ------- */
QGroupBox {{
    border: none;
    border-radius: 12px;
    background: {c("surface.card")};
    margin-top: 0;
}}
QGroupBox#primaryCard {{ background: {c("surface.card")}; }}
QGroupBox#secondaryCard {{ background: {c("surface.card")}; }}
QGroupBox#connectionReadinessCard,
QGroupBox#lastOperationCard {{
    background: {c("surface.card")};
    border: 1px solid {c("border.default")};
}}

/* ------- Buttons ------- */
QPushButton {{
    min-height: 40px;
    border-radius: 8px;
    border: none;
    color: {c("ink.inverse")};
    font-size: 13px;
    font-weight: 600;
    padding: 10px 18px;
}}
QPushButton#primaryExportButton {{
    background: {c("brand.red")};
    color: #ffffff;
    min-height: 52px;
    font-size: 14px;
    font-weight: 700;
    padding: 12px 20px;
    border-radius: 10px;
}}
QPushButton#primaryExportButton:hover {{ background: {c("brand.redHover")}; }}
QPushButton#primaryExportButton:pressed {{ background: {c("brand.redPressed")}; }}
QPushButton#primaryExportButton:disabled {{
    background: {c("status.neutralBg")};
    color: {c("ink.muted")};
}}

QPushButton#secondaryActionButton {{
    background: {c("surface.card")};
    color: {c("ink.primary")};
    border: 1px solid {c("border.strong")};
    min-height: 48px;
    font-size: 13px;
    font-weight: 600;
    padding: 10px 16px;
    border-radius: 10px;
    text-align: left;
}}
QPushButton#secondaryActionButton:hover:!disabled {{
    background: {c("surface.inset")};
    border-color: {c("ink.muted")};
    color: {c("ink.primary")};
}}
QPushButton#secondaryActionButton:pressed:!disabled {{
    background: {c("border.subtle")};
    color: {c("ink.primary")};
}}
QPushButton#secondaryActionButton:disabled {{
    background: {c("surface.subtle")};
    color: {c("ink.muted")};
    border-color: {c("border.default")};
}}

QPushButton#utilityButton {{
    background: {c("surface.card")};
    color: {c("ink.primary")};
    border: 1px solid {c("border.strong")};
    min-height: 36px;
    font-size: 12px;
    font-weight: 600;
    padding: 6px 14px;
    border-radius: 8px;
}}
QPushButton#utilityButton:hover {{
    background: {c("surface.inset")};
    border-color: {c("ink.muted")};
}}
QPushButton#utilityButton:pressed {{
    background: {c("border.subtle")};
}}

QPushButton:disabled {{
    background: {c("status.neutralBg")};
    color: {c("ink.muted")};
}}

/* ------- Inputs ------- */
QPlainTextEdit, QTextEdit, QTableWidget {{
    background: {c("surface.card")};
    border: 1px solid {c("border.default")};
    border-radius: 10px;
    color: {c("ink.primary")};
    selection-background-color: {c("brand.redTintStrong")};
    selection-color: {c("ink.primary")};
}}
QLineEdit {{
    background: {c("surface.card")};
    color: {c("ink.primary")};
    border: 1px solid {c("border.default")};
    border-radius: 8px;
    padding: 8px 12px;
    selection-background-color: {c("brand.redTintStrong")};
    selection-color: {c("ink.primary")};
}}
QLineEdit:focus {{
    border: 2px solid {c("border.focus")};
    padding: 7px 11px;
}}
QComboBox {{
    background: {c("surface.card")};
    color: {c("ink.primary")};
    border: 1px solid {c("border.default")};
    border-radius: 8px;
    padding: 8px 12px;
    min-height: 36px;
}}
QComboBox:focus {{ border: 2px solid {c("border.focus")}; padding: 7px 11px; }}
QComboBox::drop-down {{ border: none; width: 28px; }}
QComboBox QAbstractItemView {{
    background: {c("surface.card")};
    color: {c("ink.primary")};
    selection-background-color: {c("brand.redTint")};
    selection-color: {c("ink.primary")};
    border: 1px solid {c("border.default")};
    border-radius: 8px;
    padding: 4px;
}}

QHeaderView::section {{
    background: {c("surface.inset")};
    color: {c("ink.secondary")};
    padding: 10px 12px;
    border: none;
    font-size: 12px;
    font-weight: 700;
    text-transform: uppercase;
    letter-spacing: 0.6px;
}}

/* ------- Labels ------- */
QLabel#sectionTitle {{
    color: {c("ink.primary")};
    font-size: 16px;
    font-weight: 700;
    padding: 4px 0;
}}
QLabel#cardTitle {{
    color: {c("ink.primary")};
    font-size: 15px;
    font-weight: 700;
}}
QLabel#cardSubtitle {{
    color: {c("ink.secondary")};
    font-size: 13px;
    line-height: 1.5;
}}
QLabel#cardHelpText {{
    color: {c("ink.tertiary")};
    font-size: 12px;
    line-height: 1.4;
    background: {c("surface.inset")};
    border: 1px solid {c("border.default")};
    border-radius: 8px;
    padding: 10px 12px;
}}
QLabel#exportsPathLabel {{
    color: {c("ink.secondary")};
    background: {c("surface.inset")};
    border: 1px solid {c("border.default")};
    border-radius: 8px;
    padding: 10px 12px;
    font-size: 12px;
    font-family: {TOKENS["font"]["mono"]};
}}
QLabel#connItemLabel {{
    font-size: 13px;
    padding: 4px 0;
    color: {c("ink.primary")};
}}
QLabel#connectionTroubleshootLinks {{
    font-size: 12px;
    line-height: 1.5;
    color: {c("ink.tertiary")};
}}
QLabel#lastOperationBody {{
    font-size: 13px;
    line-height: 1.5;
    color: {c("ink.primary")};
}}

/* ------- Workflow banner ------- */
QLabel#workflowBanner {{
    font-size: 13px;
    font-weight: 600;
    color: {c("status.infoInk")};
    background: {c("status.infoBg")};
    border: 1px solid {c("status.infoBorder")};
    border-radius: 10px;
    padding: 10px 14px;
}}

/* ------- Flash banner ------- */
QLabel#flashBanner[severity="success"] {{
    background: {c("status.successBg")};
    color: {c("status.successInk")};
    border: 1px solid {c("status.successBorder")};
    font-weight: 600;
    font-size: 13px;
    border-radius: 10px;
    padding: 10px 16px;
}}
QLabel#flashBanner[severity="error"] {{
    background: {c("status.errorBg")};
    color: {c("status.errorInk")};
    border: 1px solid {c("status.errorBorder")};
    font-weight: 600;
    font-size: 13px;
    border-radius: 10px;
    padding: 10px 16px;
}}

/* ------- History table ------- */
QTableWidget#historyTable {{
    padding: 4px;
    gridline-color: transparent;
    selection-background-color: {c("brand.redTint")};
    selection-color: {c("ink.primary")};
    alternate-background-color: {c("surface.subtle")};
    outline: none;
}}
QTableWidget#historyTable::item {{
    border: none;
    outline: none;
    padding: 8px 10px;
}}
QTableWidget#historyTable::item:selected {{
    background: {c("brand.redTint")};
    color: {c("ink.primary")};
}}
QTableWidget#historyTable::item:hover {{
    background: {c("surface.inset")};
}}
QTableWidget#historyTable::item:selected:hover {{
    background: {c("brand.redTintStrong")};
    color: {c("ink.primary")};
}}

/* ------- History filter widgets ------- */
QLabel#filterFieldLabel {{
    color: {c("ink.tertiary")};
    font-size: 11px;
    font-weight: 700;
    letter-spacing: 0.6px;
    text-transform: uppercase;
}}
QLineEdit#historySearchField {{
    min-height: 36px;
    padding: 8px 12px;
    font-size: 13px;
}}
QComboBox#historyOutcomeFilter {{
    min-height: 36px;
    min-width: 130px;
}}

/* ------- Empty state ------- */
QWidget#emptyHistoryState {{
    background: {c("surface.subtle")};
    border: 1px dashed {c("border.strong")};
    border-radius: 12px;
    min-height: 260px;
}}
QLabel#emptyHistoryIcon {{ font-size: 40px; color: {c("ink.muted")}; }}
QLabel#emptyHistoryTitle {{
    color: {c("ink.primary")};
    font-size: 17px;
    font-weight: 700;
}}
QLabel#emptyHistoryHint {{
    color: {c("ink.tertiary")};
    font-size: 13px;
    line-height: 1.5;
    max-width: 520px;
}}

/* ------- Progress bar ------- */
QProgressBar#operationProgress {{
    background: {c("border.subtle")};
    border: none;
    border-radius: 3px;
}}
QProgressBar#operationProgress::chunk {{
    background: {c("brand.red")};
    border-radius: 3px;
}}

/* ------- Log drawer ------- */
QFrame#logDrawer {{
    background: {c("surface.card")};
    border: 1px solid {c("border.default")};
    border-radius: 12px;
}}
QWidget#logDrawerHeader {{
    background: transparent;
}}
QLabel#logDrawerTitle {{
    color: {c("ink.primary")};
    font-size: 12px;
    font-weight: 700;
    letter-spacing: 0.8px;
    text-transform: uppercase;
}}
QLabel#logCount[severity="info"] {{
    background: {c("status.neutralBg")};
    color: {c("status.neutralInk")};
    border-radius: 9px;
    padding: 1px 8px;
    font-size: 11px;
    font-weight: 700;
}}
QLabel#logCount[severity="success"] {{
    background: {c("status.successBg")};
    color: {c("status.successInk")};
    border-radius: 9px;
    padding: 1px 8px;
    font-size: 11px;
    font-weight: 700;
}}
QLabel#logCount[severity="error"] {{
    background: {c("status.errorBg")};
    color: {c("status.errorInk")};
    border-radius: 9px;
    padding: 1px 8px;
    font-size: 11px;
    font-weight: 700;
}}
QTextEdit#statusPanel {{
    background: {c("surface.card")};
    border: none;
    border-top: 1px solid {c("border.subtle")};
    border-bottom-left-radius: 12px;
    border-bottom-right-radius: 12px;
    padding: 10px 14px;
    font-family: {TOKENS["font"]["mono"]};
    font-size: 12px;
    color: {c("ink.primary")};
}}
QPushButton#statusMiniButton {{
    min-height: 26px;
    min-width: 58px;
    padding: 4px 10px;
    font-size: 11px;
    font-weight: 600;
    background: {c("surface.card")};
    color: {c("ink.secondary")};
    border: 1px solid {c("border.default")};
    border-radius: 6px;
}}
QPushButton#statusMiniButton:hover {{
    background: {c("surface.inset")};
    color: {c("ink.primary")};
    border-color: {c("ink.muted")};
}}
QPushButton#drawerToggle {{
    background: transparent;
    border: none;
    color: {c("ink.tertiary")};
    padding: 4px 8px;
    min-width: 28px;
    min-height: 26px;
    border-radius: 6px;
}}
QPushButton#drawerToggle:hover {{
    background: {c("surface.inset")};
    color: {c("ink.primary")};
}}

/* ------- Step progress ------- */
QWidget#stepProgressBar {{
    background: transparent;
}}

/* ------- Dialogs ------- */
QDialog {{ background: {c("surface.card")}; color: {c("ink.primary")}; }}
QFrame#uploadConfirmBox {{
    background: {c("surface.subtle")};
    border: 1px solid {c("border.default")};
    border-radius: 10px;
}}
QFrame#uploadErrorBox {{
    background: {c("status.errorBg")};
    border: 1px solid {c("status.errorBorder")};
    border-radius: 10px;
}}
QLabel#uploadErrorIcon {{
    background: {c("brand.red")};
    color: #ffffff;
    border-radius: 22px;
    font-size: 22px;
    font-weight: 800;
}}
QLabel#uploadErrorMessage {{
    color: {c("ink.primary")};
    font-size: 14px;
    line-height: 1.5;
}}

QMessageBox {{ background: {c("surface.card")}; }}
QMessageBox QLabel {{
    color: {c("ink.primary")};
    font-size: 14px;
    line-height: 1.45;
    min-width: 280px;
}}
QMessageBox QPushButton {{
    min-height: 36px;
    min-width: 92px;
    background-color: {c("ink.primary")};
    color: #ffffff;
    border-radius: 8px;
    font-size: 13px;
    font-weight: 600;
    padding: 8px 18px;
}}
QMessageBox QPushButton:hover {{ background-color: {c("ink.secondary")}; }}
QMessageBox QPushButton:pressed {{ background-color: #000000; }}

/* ------- Checkboxes ------- */
QCheckBox {{ color: {c("ink.primary")}; spacing: 8px; }}
QCheckBox::indicator {{
    width: 18px;
    height: 18px;
    border: 2px solid {c("border.strong")};
    border-radius: 4px;
    background: {c("surface.card")};
}}
QCheckBox::indicator:hover {{ border-color: {c("brand.red")}; }}
QCheckBox::indicator:checked {{
    background: {c("brand.red")};
    border-color: {c("brand.red")};
}}

/* ------- Scrollbars ------- */
QScrollBar:vertical {{
    background: transparent;
    width: 10px;
    margin: 0;
}}
QScrollBar::handle:vertical {{
    background: {c("border.strong")};
    border-radius: 5px;
    min-height: 30px;
}}
QScrollBar::handle:vertical:hover {{ background: {c("ink.muted")}; }}
QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical {{ height: 0; }}
QScrollBar:horizontal {{
    background: transparent;
    height: 10px;
    margin: 0;
}}
QScrollBar::handle:horizontal {{
    background: {c("border.strong")};
    border-radius: 5px;
    min-width: 30px;
}}
QScrollBar::handle:horizontal:hover {{ background: {c("ink.muted")}; }}
QScrollBar::add-line:horizontal,
QScrollBar::sub-line:horizontal {{ width: 0; }}
"""


# ---------------------------------------------------------------------------
#  Dark-theme stylesheet (kept simple; derived from dark tokens)
# ---------------------------------------------------------------------------


def build_dark_stylesheet() -> str:
    """Return a dark variant of the app stylesheet.

    The dark palette mirrors the light palette but with inverted surface/ink
    values; components keep the same objectNames so nothing else changes.
    """
    d = {
        "app": "#1b1d22",
        "card": "#23262c",
        "subtle": "#1f2127",
        "inset": "#2a2d33",
        "primary": "#f5f6f8",
        "secondary": "#c0c6cf",
        "tertiary": "#9aa2ad",
        "muted": "#6b7280",
        "border": "#32363d",
        "borderStrong": "#40454d",
        "borderSubtle": "#2b2e34",
        "brandRed": TOKENS["brand"]["red"],  # type: ignore[index]
        "brandRedHover": TOKENS["brand"]["redHover"],  # type: ignore[index]
        "brandRedAccent": TOKENS["brand"]["redAccent"],  # type: ignore[index]
        "brandTint": "#3a1d26",
        "brandTintStrong": "#4a1f2c",
        "successBg": "#1f3b28",
        "successInk": "#86efac",
        "successBorder": "#2a5336",
        "errorBg": "#3b1f23",
        "errorInk": "#fca5a5",
        "errorBorder": "#5a2a30",
        "infoBg": "#1e2f4b",
        "infoInk": "#93c5fd",
        "infoBorder": "#30466d",
        "neutralBg": "#2a2d33",
        "neutralInk": "#c0c6cf",
        "neutralBorder": "#40454d",
    }
    return f"""
* {{ font-family: {TOKENS["font"]["family"]}; }}
QWidget#appRoot {{ background: {d["app"]}; color: {d["primary"]}; font-size: 14px; }}
QWidget#masthead {{ background: {d["card"]}; border: none; border-radius: 12px; }}
QWidget#mastheadContent {{ background: transparent; }}
QLabel#brandLogo {{ background: transparent; }}
QLabel#brandFallback {{ color: {d["brandRedAccent"]}; font-size: 28px; font-weight: 900; font-style: italic; letter-spacing: -0.5px; }}
QFrame#mastheadDivider {{ background: {d["border"]}; border: none; }}
QLabel#eyebrowLabel {{ color: {d["brandRedAccent"]}; font-size: 10px; font-weight: 700; letter-spacing: 1.4px; text-transform: uppercase; }}
QLabel#heroTitle {{ color: {d["primary"]}; font-size: 22px; font-weight: 700; }}
QLabel#heroSubtitle {{ color: {d["tertiary"]}; font-size: 13px; max-width: 880px; }}
QFrame#accentLine {{ background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 {d["brandRed"]}, stop:1 {d["brandRedAccent"]}); border: none; border-bottom-left-radius: 12px; border-bottom-right-radius: 12px; }}

QFrame#sidebar {{ background: {d["card"]}; border: none; border-radius: 12px; }}
QLabel#sidebarBrand {{ color: {d["brandRedAccent"]}; font-size: 11px; font-weight: 800; letter-spacing: 1.6px; padding: 6px 14px 0 14px; }}
QWidget#sidebarDivider {{ background: {d["borderSubtle"]}; max-height: 1px; min-height: 1px; }}
QPushButton#sidebarItem {{ text-align: left; padding: 10px 16px 10px 14px; min-height: 40px; border: none; border-radius: 8px; color: {d["secondary"]}; background: transparent; font-size: 13px; font-weight: 600; }}
QPushButton#sidebarItem:hover {{ background: {d["inset"]}; color: {d["primary"]}; }}
QPushButton#sidebarItem[active="true"] {{ background: {d["brandTint"]}; color: {d["brandRedAccent"]}; }}
QPushButton#sidebarCollapseToggle {{ text-align: left; padding: 8px 14px; min-height: 36px; border: none; border-radius: 8px; color: {d["tertiary"]}; background: transparent; font-size: 12px; font-weight: 600; }}
QPushButton#sidebarCollapseToggle:hover {{ background: {d["inset"]}; color: {d["primary"]}; }}
QLabel#sidebarBadge {{ background: {d["brandTint"]}; color: {d["brandRedAccent"]}; border-radius: 9px; padding: 1px 7px; font-size: 10px; font-weight: 800; min-width: 16px; }}

QGroupBox {{ border: none; border-radius: 12px; background: {d["card"]}; margin-top: 0; }}
QGroupBox#connectionReadinessCard, QGroupBox#lastOperationCard {{ background: {d["card"]}; border: 1px solid {d["border"]}; }}

QPushButton {{ min-height: 40px; border-radius: 8px; border: none; color: #ffffff; font-size: 13px; font-weight: 600; padding: 10px 18px; }}
QPushButton#primaryExportButton {{ background: {d["brandRed"]}; color: #ffffff; min-height: 52px; font-size: 14px; font-weight: 700; padding: 12px 20px; border-radius: 10px; }}
QPushButton#primaryExportButton:hover {{ background: {d["brandRedHover"]}; }}
QPushButton#primaryExportButton:disabled {{ background: {d["neutralBg"]}; color: {d["muted"]}; }}

QPushButton#secondaryActionButton {{ background: {d["card"]}; color: {d["primary"]}; border: 1px solid {d["borderStrong"]}; min-height: 48px; font-size: 13px; font-weight: 600; padding: 10px 16px; border-radius: 10px; text-align: left; }}
QPushButton#secondaryActionButton:hover:!disabled {{ background: {d["inset"]}; border-color: {d["muted"]}; }}
QPushButton#secondaryActionButton:disabled {{ background: {d["subtle"]}; color: {d["muted"]}; border-color: {d["border"]}; }}

QPushButton#utilityButton {{ background: {d["card"]}; color: {d["primary"]}; border: 1px solid {d["borderStrong"]}; min-height: 36px; font-size: 12px; font-weight: 600; padding: 6px 14px; border-radius: 8px; }}
QPushButton#utilityButton:hover {{ background: {d["inset"]}; border-color: {d["muted"]}; }}

QPushButton:disabled {{ background: {d["neutralBg"]}; color: {d["muted"]}; }}

QPlainTextEdit, QTextEdit, QTableWidget {{ background: {d["card"]}; border: 1px solid {d["border"]}; border-radius: 10px; color: {d["primary"]}; selection-background-color: {d["brandTintStrong"]}; selection-color: {d["primary"]}; }}
QLineEdit {{ background: {d["card"]}; color: {d["primary"]}; border: 1px solid {d["border"]}; border-radius: 8px; padding: 8px 12px; }}
QLineEdit:focus {{ border: 2px solid {d["brandRedAccent"]}; padding: 7px 11px; }}
QComboBox {{ background: {d["card"]}; color: {d["primary"]}; border: 1px solid {d["border"]}; border-radius: 8px; padding: 8px 12px; min-height: 36px; }}
QComboBox::drop-down {{ border: none; width: 28px; }}
QComboBox QAbstractItemView {{ background: {d["card"]}; color: {d["primary"]}; selection-background-color: {d["brandTint"]}; selection-color: {d["primary"]}; border: 1px solid {d["border"]}; border-radius: 8px; padding: 4px; }}
QHeaderView::section {{ background: {d["inset"]}; color: {d["tertiary"]}; padding: 10px 12px; border: none; font-size: 12px; font-weight: 700; text-transform: uppercase; letter-spacing: 0.6px; }}

QLabel#sectionTitle {{ color: {d["primary"]}; font-size: 16px; font-weight: 700; padding: 4px 0; }}
QLabel#cardTitle {{ color: {d["primary"]}; font-size: 15px; font-weight: 700; }}
QLabel#cardSubtitle {{ color: {d["tertiary"]}; font-size: 13px; line-height: 1.5; }}
QLabel#exportsPathLabel {{ color: {d["tertiary"]}; background: {d["inset"]}; border: 1px solid {d["border"]}; border-radius: 8px; padding: 10px 12px; font-size: 12px; font-family: {TOKENS["font"]["mono"]}; }}
QLabel#connItemLabel {{ font-size: 13px; padding: 4px 0; color: {d["primary"]}; }}
QLabel#connectionTroubleshootLinks {{ font-size: 12px; line-height: 1.5; color: {d["tertiary"]}; }}
QLabel#lastOperationBody {{ font-size: 13px; line-height: 1.5; color: {d["primary"]}; }}

QLabel#workflowBanner {{ font-size: 13px; font-weight: 600; color: {d["infoInk"]}; background: {d["infoBg"]}; border: 1px solid {d["infoBorder"]}; border-radius: 10px; padding: 10px 14px; }}

QLabel#flashBanner[severity="success"] {{ background: {d["successBg"]}; color: {d["successInk"]}; border: 1px solid {d["successBorder"]}; font-weight: 600; font-size: 13px; border-radius: 10px; padding: 10px 16px; }}
QLabel#flashBanner[severity="error"] {{ background: {d["errorBg"]}; color: {d["errorInk"]}; border: 1px solid {d["errorBorder"]}; font-weight: 600; font-size: 13px; border-radius: 10px; padding: 10px 16px; }}

QTableWidget#historyTable {{ padding: 4px; gridline-color: transparent; selection-background-color: {d["brandTint"]}; selection-color: {d["primary"]}; alternate-background-color: {d["subtle"]}; outline: none; }}
QTableWidget#historyTable::item {{ border: none; outline: none; padding: 8px 10px; }}
QTableWidget#historyTable::item:hover {{ background: {d["inset"]}; }}

QLabel#filterFieldLabel {{ color: {d["tertiary"]}; font-size: 11px; font-weight: 700; letter-spacing: 0.6px; text-transform: uppercase; }}

QWidget#emptyHistoryState {{ background: {d["subtle"]}; border: 1px dashed {d["borderStrong"]}; border-radius: 12px; min-height: 260px; }}
QLabel#emptyHistoryIcon {{ font-size: 40px; color: {d["muted"]}; }}
QLabel#emptyHistoryTitle {{ color: {d["primary"]}; font-size: 17px; font-weight: 700; }}
QLabel#emptyHistoryHint {{ color: {d["tertiary"]}; font-size: 13px; line-height: 1.5; max-width: 520px; }}

QProgressBar#operationProgress {{ background: {d["borderSubtle"]}; border: none; border-radius: 3px; }}
QProgressBar#operationProgress::chunk {{ background: {d["brandRedAccent"]}; border-radius: 3px; }}

QFrame#logDrawer {{ background: {d["card"]}; border: 1px solid {d["border"]}; border-radius: 12px; }}
QWidget#logDrawerHeader {{ background: transparent; }}
QLabel#logDrawerTitle {{ color: {d["primary"]}; font-size: 12px; font-weight: 700; letter-spacing: 0.8px; text-transform: uppercase; }}
QLabel#logCount[severity="info"] {{ background: {d["neutralBg"]}; color: {d["neutralInk"]}; border-radius: 9px; padding: 1px 8px; font-size: 11px; font-weight: 700; }}
QLabel#logCount[severity="success"] {{ background: {d["successBg"]}; color: {d["successInk"]}; border-radius: 9px; padding: 1px 8px; font-size: 11px; font-weight: 700; }}
QLabel#logCount[severity="error"] {{ background: {d["errorBg"]}; color: {d["errorInk"]}; border-radius: 9px; padding: 1px 8px; font-size: 11px; font-weight: 700; }}
QTextEdit#statusPanel {{ background: {d["card"]}; border: none; border-top: 1px solid {d["borderSubtle"]}; border-bottom-left-radius: 12px; border-bottom-right-radius: 12px; padding: 10px 14px; font-family: {TOKENS["font"]["mono"]}; font-size: 12px; color: {d["primary"]}; }}
QPushButton#statusMiniButton {{ min-height: 26px; min-width: 58px; padding: 4px 10px; font-size: 11px; font-weight: 600; background: {d["card"]}; color: {d["tertiary"]}; border: 1px solid {d["border"]}; border-radius: 6px; }}
QPushButton#statusMiniButton:hover {{ background: {d["inset"]}; color: {d["primary"]}; border-color: {d["muted"]}; }}
QPushButton#drawerToggle {{ background: transparent; border: none; color: {d["tertiary"]}; padding: 4px 8px; min-width: 28px; min-height: 26px; border-radius: 6px; }}
QPushButton#drawerToggle:hover {{ background: {d["inset"]}; color: {d["primary"]}; }}

QDialog {{ background: {d["card"]}; color: {d["primary"]}; }}
QFrame#uploadConfirmBox {{ background: {d["subtle"]}; border: 1px solid {d["border"]}; border-radius: 10px; }}
QFrame#uploadErrorBox {{ background: {d["errorBg"]}; border: 1px solid {d["errorBorder"]}; border-radius: 10px; }}
QLabel#uploadErrorIcon {{ background: {d["brandRed"]}; color: #ffffff; border-radius: 22px; font-size: 22px; font-weight: 800; }}
QLabel#uploadErrorMessage {{ color: {d["primary"]}; font-size: 14px; line-height: 1.5; }}

QMessageBox {{ background: {d["card"]}; }}
QMessageBox QLabel {{ color: {d["primary"]}; font-size: 14px; line-height: 1.45; min-width: 280px; }}
QMessageBox QPushButton {{ min-height: 36px; min-width: 92px; background-color: {d["primary"]}; color: {d["card"]}; border-radius: 8px; font-size: 13px; font-weight: 600; padding: 8px 18px; }}

QCheckBox {{ color: {d["primary"]}; spacing: 8px; }}
QCheckBox::indicator {{ width: 18px; height: 18px; border: 2px solid {d["borderStrong"]}; border-radius: 4px; background: {d["card"]}; }}
QCheckBox::indicator:checked {{ background: {d["brandRedAccent"]}; border-color: {d["brandRedAccent"]}; }}

QScrollBar:vertical {{ background: transparent; width: 10px; margin: 0; }}
QScrollBar::handle:vertical {{ background: {d["borderStrong"]}; border-radius: 5px; min-height: 30px; }}
QScrollBar::handle:vertical:hover {{ background: {d["muted"]}; }}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {{ height: 0; }}
"""
