# EOL Export Software

Desktop software for the Rite-Hite EOL test fixture. This application handles:

- manual CSV export from the STM32 fixture
- Rite-Hite Connect test-firmware upload
- the One Click workflow that uploads firmware, starts the automatic test, tracks live progress, and exports the final CSV

## Recommended Install

For operators and test stations, use the packaged Windows installer rather than running from source.

Build the installer from the repository root:

```powershell
cd C:\TouchGFXProjects\EOL_TestFixture_Final
cmd /c Export_Software\build_installer.bat
```

Installer output:

```text
Export_Software\dist\installer\
```

The installer includes:

- the packaged desktop app
- the Python runtime and Qt dependencies
- the bundled ESP32 flashing tool
- the prebuilt ESP32 test firmware bundle
- optional Word documentation

Installed builds store writable runtime data in:

```text
%LOCALAPPDATA%\Rite-Hite\EOL Export Software
```

## Run From Source

For development:

```powershell
cd C:\TouchGFXProjects\EOL_TestFixture_Final\Export_Software
python -m pip install -r requirements.txt
python -m pip install pyinstaller
python main.py
```

## Build Packaged App Only

If you only want the unpacked PyInstaller folder and not the installer:

```powershell
cd C:\TouchGFXProjects\EOL_TestFixture_Final\Export_Software
.\build_export_software.bat
```

Output:

```text
Export_Software\dist\EOL_Export_Software\
```

## Notes

- The export path uses the STM32 ST-LINK virtual COM port at `115200 8N1`.
- Export is available only after a test reaches a terminal pass or fail state.
- Each completed run can be exported once.
- The installed app does not require a separate Arduino IDE installation for ESP32 test-firmware upload.
- Source-mode exports default to `Export_Software\exports`.
