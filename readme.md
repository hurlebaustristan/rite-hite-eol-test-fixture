# Rite-Hite EOL Test Fixture

End-of-line test fixture firmware and desktop tooling for the Rite-Hite Connect module.

This repository contains:

- STM32U575 firmware for the touchscreen-driven test fixture
- TouchGFX UI for running and visualizing fixture tests
- PySide6 desktop software for firmware upload, CSV export, and One Click automated test flow
- ESP32 test firmware assets used by the desktop uploader

## Repository Layout

- `Core/`: STM32 application logic, test engines, UART export protocol, and auto-test state machine
- `TouchGFX/`: TouchGFX project, generated UI code, assets, simulator files, and screen presenters/views
- `Export_Software/`: PySide6 desktop app, PyInstaller packaging files, translations, widgets, and export workflow
- `STM32CubeIDE/`, `gcc/`, `EWARM/`, `MDK-ARM/`: embedded build targets and IDE/toolchain project files
- `ArduinoIDE/`: ESP32-side firmware content used by the fixture workflow
- `Docs/`: project-specific documentation and reference material

## Major Workflows

### Firmware and TouchGFX

Use TouchGFX Designer or STM32CubeIDE to regenerate and build the embedded project.

- TouchGFX target board: `STM32U575ZI_NUCLEO_RVA35HI`
- Main firmware entry/config: `Core/`, `TouchGFX/`, `STM32CubeIDE/`
- One Click firmware path supports:
  - auto-run of fixture tests
  - export readiness polling
  - technician LED visual confirmation

### Desktop Export Software

Install dependencies:

```powershell
cd C:\TouchGFXProjects\EOL_TestFixture_Final\Export_Software
python -m pip install -r requirements.txt
```

Run the desktop app:

```powershell
cd C:\TouchGFXProjects\EOL_TestFixture_Final\Export_Software
python main.py
```

Build the Windows executable:

```powershell
cd C:\TouchGFXProjects\EOL_TestFixture_Final\Export_Software
.\build_export_software.bat
```

The packaged application is created under `Export_Software\dist\EOL_Export_Software\`.

## Notes

- The fixture communicates with the PC over the STM32 ST-LINK virtual COM port.
- CSV exports are written to `Export_Software\exports\`.
- The desktop app also packages the ESP32 test firmware assets needed for upload.
- Build artifacts, runtime exports, and packaged binaries are intentionally ignored by Git.
