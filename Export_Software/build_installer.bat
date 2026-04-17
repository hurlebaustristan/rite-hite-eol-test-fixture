@echo off
setlocal
cd /d "%~dp0"

for /f "usebackq delims=" %%i in (`python -c "from release_info import APP_VERSION; print(APP_VERSION)"`) do set "APP_VERSION=%%i"
if not defined APP_VERSION (
    echo Failed to read APP_VERSION from release_info.py
    exit /b 1
)

call "%~dp0build_export_software.bat"
if errorlevel 1 (
    exit /b 1
)

set "ISCC_EXE="
for %%p in (
    "%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe"
    "%ProgramFiles%\Inno Setup 6\ISCC.exe"
    "%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe"
) do (
    if exist %%~p set "ISCC_EXE=%%~p"
)

if not defined ISCC_EXE (
    for /f "delims=" %%p in ('where iscc.exe 2^>nul') do (
        set "ISCC_EXE=%%p"
        goto found_iscc
    )
)

:found_iscc
if not defined ISCC_EXE (
    echo Inno Setup was not found.
    echo Install it with: winget install --id JRSoftware.InnoSetup -e
    exit /b 1
)

"%ISCC_EXE%" /DMyAppVersion=%APP_VERSION% "%~dp0installer\EOL_Export_Software.iss"
if errorlevel 1 (
    echo.
    echo Installer build failed.
    exit /b 1
)

echo.
echo Installer build complete.
echo Output folder: %~dp0dist\installer
echo Installer file: %~dp0dist\installer\EOL_Export_Software_Setup_%APP_VERSION%.exe
endlocal
