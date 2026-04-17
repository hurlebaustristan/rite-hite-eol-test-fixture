@echo off
setlocal
cd /d "%~dp0"
python -m PyInstaller --noconfirm export_software.spec
if errorlevel 1 (
    echo.
    echo Build failed.
    exit /b 1
)
echo.
echo Build complete.
echo Output folder: %~dp0dist\EOL_Export_Software
endlocal
