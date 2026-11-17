[Setup]
AppId={{8F4E9B2A-5C1D-4E7A-9B3F-1A2B3C4D5E6F}
AppName=PS Taskbar Widget
AppVersion=1.0.0
AppPublisher=PSVis
DefaultDirName={autopf}\PsWidget
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
OutputDir=.
OutputBaseFilename=PsWidget_Setup
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
CloseApplications=force

[Languages]
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"

[Tasks]
Name: "autostart"; Description: "Widget autostart"; GroupDescription: "Launch options:"
Name: "psprofile"; Description: "Automatically connect ($PROFILE)"; GroupDescription: "Terminal integration:"

[Files]
Source: "PsWidget.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "pswidget-hook.ps1"; DestDir: "{app}"; Flags: ignoreversion
Source: "manage-profile.ps1"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\PS Taskbar Widget"; Filename: "{app}\PsWidget.exe"

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "PsWidget"; ValueData: """{app}\PsWidget.exe"""; Flags: uninsdeletevalue; Tasks: autostart

[Run]
Filename: "powershell.exe"; Parameters: "-NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File ""{app}\manage-profile.ps1"" -Action Install -HookPath ""{app}\pswidget-hook.ps1"""; Flags: runhidden waituntilterminated; Tasks: psprofile

Filename: "{app}\PsWidget.exe"; Description: "Uruchom PS Taskbar Widget teraz"; Flags: nowait postinstall skipifsilent

[UninstallRun]

Filename: "taskkill.exe"; Parameters: "/F /IM PsWidget.exe"; Flags: runhidden; RunOnceId: "KillWidget"

Filename: "powershell.exe"; Parameters: "-NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File ""{app}\manage-profile.ps1"" -Action Uninstall -HookPath ""{app}\pswidget-hook.ps1"""; Flags: runhidden waituntilterminated; RunOnceId: "RemoveProfileHook"