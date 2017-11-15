; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Forgotten Wars"
#define MyAppVersion "1.0"
#define MyAppPublisher "Trill.Net"
#define MyAppURL "https://trill.net/"
#define MyAppExeName "fw.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{7E52C819-379E-4BA4-A481-DDEBC0742D0A}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=C:\Users\Stefan Ascher\Documents\Visual Studio 2015\Projects\ABx\abclient\LICENSE.txt
OutputDir=c:\Users\Stefan Ascher\Documents\Visual Studio 2015\Projects\ABx\abclient\Setup\
OutputBaseFilename=fwclient-setup
SetupIconFile=C:\Users\Stefan Ascher\Documents\Visual Studio 2015\Projects\ABx\abclient\abclient\abclient.ico
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64 

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Users\Stefan Ascher\Documents\Visual Studio 2015\Projects\ABx\abclient\bin\fw.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stefan Ascher\Documents\Visual Studio 2015\Projects\ABx\abclient\Setup\AbData.pak"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\Users\Stefan Ascher\Documents\Visual Studio 2015\Projects\ABx\abclient\Setup\Autoload.pak"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\Users\Stefan Ascher\Documents\Visual Studio 2015\Projects\ABx\abclient\Setup\CoreData.pak"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\Users\Stefan Ascher\Documents\Visual Studio 2015\Projects\ABx\abclient\Setup\Data.pak"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\Users\Stefan Ascher\Documents\Visual Studio 2015\Projects\ABx\abclient\bin\d3dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{commonprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

