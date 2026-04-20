[Setup]
AppName=Pour
AppVersion=1.0.0
AppVerName=Pour v1.0.0
AppPublisher=Carbonated Audio
AppPublisherURL=https://carbonatedaudio.com
DefaultDirName={autopf}\Carbonated Audio\Pour
DefaultGroupName=Carbonated Audio
OutputDir=..\..\dist\Windows\Installer
OutputBaseFilename=Pour-v1.0.0-Windows-Installer
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
DisableProgramGroupPage=yes
PrivilegesRequired=admin

[Files]
; VST3
Source: "..\..\dist\Windows\VST3\Pour.vst3\*"; DestDir: "{commoncf64}\VST3\Pour.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

; Standalone
Source: "..\..\dist\Windows\Standalone\Pour.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\Pour"; Filename: "{app}\Pour.exe"
Name: "{group}\Uninstall Pour"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\Pour.exe"; Description: "Launch Pour"; Flags: nowait postinstall skipifsilent
