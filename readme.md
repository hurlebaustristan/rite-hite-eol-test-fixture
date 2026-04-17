# Rite-Hite EOL Test Fixture

End-of-line test fixture firmware and desktop tooling for the Rite-Hite Connect module.

This repository contains:

- STM32U575 firmware for the touchscreen-driven test fixture
- TouchGFX UI for running and visualizing fixture tests
- PySide6 desktop software for firmware upload, CSV export, and the One Click automated test flow
- bundled ESP32 test firmware and flashing tools used by the desktop uploader
- operator and engineering documentation under `Docs/`

## Repository Layout

- `Core/`: STM32 application logic, test engines, UART export protocol, and auto-test state machine
- `TouchGFX/`: TouchGFX project, generated UI code, assets, simulator files, and screen presenters/views
- `Export_Software/`: PySide6 desktop app, packaging files, installer files, translations, widgets, and export workflow
- `STM32CubeIDE/`, `gcc/`, `EWARM/`, `MDK-ARM/`: embedded build targets and IDE/toolchain project files
- `ArduinoIDE/`: ESP32-side firmware source used by the fixture workflow
- `Docs/`: project-specific documentation and reference material

## Operator Install On Windows

For a normal laptop or test station, the recommended install path is the Windows installer for the desktop app.

What the installer includes:

- the packaged `EOL Export Software` desktop application
- the Python runtime and Qt dependencies required to run it
- the bundled ESP32 flashing tool and prebuilt ESP32 test firmware
- optional Word documentation copied into the install folder

What it does not install:

- USB drivers, if the laptop does not already recognize the fixture/programmer COM ports
- Microsoft Word, if you want to open the bundled `.docx` files directly
- the embedded STM32 firmware itself onto the fixture hardware

Runtime data for an installed build is stored per user under:

```text
%LOCALAPPDATA%\Rite-Hite\EOL Export Software
```

That folder holds:

- `exports\`
- `settings.json`
- `last_successful_operation.json`

## Build The Desktop Installer

If you clone this repository and want to build the installer yourself:

1. Install Python 3.12 or newer.
2. Install Python dependencies:

```powershell
cd C:\TouchGFXProjects\EOL_TestFixture_Final\Export_Software
python -m pip install -r requirements.txt
python -m pip install pyinstaller
```

3. Install Inno Setup 6:

```powershell
winget install --id JRSoftware.InnoSetup -e
```

4. Build the packaged app and installer:

```powershell
cd C:\TouchGFXProjects\EOL_TestFixture_Final
cmd /c Export_Software\build_installer.bat
```

Installer output:

```text
Export_Software\dist\installer\
```

Packaged app output:

```text
Export_Software\dist\EOL_Export_Software\
```

## Developer Run From Source

If you are developing the desktop app instead of installing it:

```powershell
cd C:\TouchGFXProjects\EOL_TestFixture_Final\Export_Software
python -m pip install -r requirements.txt
python main.py
```

Source-mode runtime files are written under:

```text
Export_Software\
```

## Firmware And TouchGFX

Use TouchGFX Designer or STM32CubeIDE to regenerate and build the embedded project.

- TouchGFX target board: `STM32U575ZI_NUCLEO_RVA35HI`
- Main firmware entry/config: `Core/`, `TouchGFX/`, `STM32CubeIDE/`
- One Click firmware path supports:
  - automatic stage progression through the production tests
  - status polling from the desktop app
  - technician LED visual confirmation at the final stage

## Desktop App Notes

- The fixture communicates with the PC over the STM32 ST-LINK virtual COM port.
- The desktop app can upload the Rite-Hite Connect test firmware without requiring a separate Arduino IDE install.
- The One Click flow uploads firmware, starts the automatic run, tracks live progress, waits for the LED decision, and exports the CSV automatically.
- Build artifacts, runtime exports, and packaged binaries are intentionally ignored by Git.
