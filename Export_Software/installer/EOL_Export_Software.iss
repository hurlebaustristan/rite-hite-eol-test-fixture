#ifndef MyAppVersion
  #define MyAppVersion "2026.4.17.2"
#endif

#define MyAppName "EOL Export Software"
#define MyAppPublisher "Rite-Hite"
#define MyAppExeName "EOL Export Software.exe"
#define MyAppId "{{8C75F699-3D02-4E4F-B3B8-D8E3CFB4B4A8}"
#define MyAppDir "..\\dist\\EOL_Export_Software"
#define MyDocsDir "..\\..\\Docs\\Word"
#define MyIcon "..\\assets\\rite_hite_app_icon.ico"
#define MyOutputDir "..\\dist\\installer"

[Setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={localappdata}\Programs\Rite-Hite\{#MyAppName}
DefaultGroupName=Rite-Hite\{#MyAppName}
DisableProgramGroupPage=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir={#MyOutputDir}
OutputBaseFilename=EOL_Export_Software_Setup_{#MyAppVersion}
SetupIconFile={#MyIcon}
UninstallDisplayIcon={app}\{#MyAppExeName}
WizardStyle=modern
Compression=lzma2/ultra64
SolidCompression=yes
ChangesAssociations=no
UsePreviousAppDir=yes
CloseApplications=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "full"; Description: "Full installation"
Name: "compact"; Description: "Application only"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "main"; Description: "EOL Export Software application"; Types: full compact custom; Flags: fixed
Name: "docs"; Description: "Fixture documentation (Word documents)"; Types: full custom

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional shortcuts:"; Flags: unchecked

[Files]
Source: "{#MyAppDir}\\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: main
Source: "{#MyDocsDir}\\*.docx"; DestDir: "{app}\Documentation"; Flags: ignoreversion; Components: docs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Components: main
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"; Components: main
Name: "{group}\EOL Test Fixture User Manual"; Filename: "{app}\Documentation\user_manual.docx"; Components: docs
Name: "{group}\Documentation Folder"; Filename: "{app}\Documentation"; Components: docs
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon; Components: main

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch {#MyAppName}"; Flags: nowait postinstall skipifsilent; Components: main
