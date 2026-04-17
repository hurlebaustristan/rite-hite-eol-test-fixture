# -*- mode: python ; coding: utf-8 -*-

from pathlib import Path

from PyInstaller.utils.hooks import collect_data_files


project_dir = Path(SPECPATH)
project_root = project_dir.parent
icon_path = project_dir / "assets" / "rite_hite_app_icon.ico"

datas = [
    (str(project_dir / "assets"), "assets"),
    (str(project_dir / "esp32_test_firmware"), "esp32_test_firmware"),
    (str(project_dir / "tools"), "tools"),
]

# Bundle the qtawesome icon fonts (FontAwesome / MaterialDesignIcons) with the
# executable so the sidebar/drawer/table icons render correctly on every
# machine the fixture software is deployed to.
datas += collect_data_files("qtawesome")


a = Analysis(
    [str(project_dir / "main.py")],
    pathex=[str(project_dir)],
    binaries=[],
    datas=datas,
    hiddenimports=["PySide6.QtSvgWidgets", "qtawesome"],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)

pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    [],
    exclude_binaries=True,
    name="EOL_Export_Software",
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon=str(icon_path) if icon_path.exists() else None,
)

coll = COLLECT(
    exe,
    a.binaries,
    a.datas,
    strip=False,
    upx=True,
    upx_exclude=[],
    name="EOL_Export_Software",
)
