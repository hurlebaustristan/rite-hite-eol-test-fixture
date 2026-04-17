# EOL Export Software

Desktop software for the Rite-Hite EOL test fixture. This application handles:

- manual CSV export from the STM32 fixture
- Rite-Hite Connect test-firmware upload
- the One Click workflow that uploads firmware, starts the automatic test, tracks live progress, and exports the final CSV

## Recommended Install

For operators and test stations, use the packaged Windows installer rather than running from source.

### Download From GitHub

If you are not building from source, download the installer from the repository Releases page:

```text
https://github.com/hurlebaustristan/rite-hite-eol-test-fixture/releases/latest
```

Use the installer asset named:

```text
EOL_Export_Software_Setup_2026.4.17.3.exe
```

### Exact Installer File

Current installer build:

```text
Export_Software\dist\installer\EOL_Export_Software_Setup_2026.4.17.3.exe
```

If you are already inside this repository on Windows, the full path is:

```text
C:\TouchGFXProjects\EOL_TestFixture_Final\Export_Software\dist\installer\EOL_Export_Software_Setup_2026.4.17.3.exe
```

### How To Install

1. Double-click `EOL_Export_Software_Setup_2026.4.17.3.exe`.
2. Accept the installer prompts.
3. Keep the default install folder unless you have a local policy that says otherwise.
4. Leave the main application component enabled.
5. Optionally leave the documentation component enabled if you want the Word manuals copied into the install.
6. Finish the install and launch the app from the Start Menu shortcut `Rite-Hite > EOL_Export_Software` or the optional desktop shortcut.

### What To Hand To Another User

If you are distributing this software to an operator or another engineer, send them the installer `.exe` above. Do not send the raw `dist\EOL_Export_Software\` folder unless they specifically need the unpacked build for debugging or packaging work.

Build the installer from the repository root:

```powershell
cd C:\TouchGFXProjects\EOL_TestFixture_Final
cmd /c Export_Software\build_installer.bat
```

After the build completes, the installer will be here:

```text
Export_Software\dist\installer\EOL_Export_Software_Setup_2026.4.17.3.exe
```

The installer includes:

- the packaged desktop app
- the Python runtime and Qt dependencies
- the bundled ESP32 flashing tool
- the prebuilt ESP32 test firmware bundle
- optional Word documentation

Installed builds store writable runtime data in:

```text
%LOCALAPPDATA%\Rite-Hite\EOL_Export_Software
```

That is also the first place to check for generated CSV exports and saved runtime state on an installed machine.

If you are upgrading from an older install, the app will try to move the legacy runtime-data folder with spaces into the new underscore-based folder on first launch.

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
