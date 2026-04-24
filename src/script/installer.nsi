Unicode True

; Force 64-bit install
!include "x64.nsh"
RequestExecutionLevel admin

Function .onInit
    ${If} ${RunningX64}
        SetRegView 64
    ${EndIf}
FunctionEnd
;---------------------------------------------------------
; General settings
;---------------------------------------------------------
Name        "fOptics"
OutFile     "fOptics_Installer.exe"
InstallDir  "$PROGRAMFILES64\fOptics"

;---------------------------------------------------------
; Modern UI pages
;---------------------------------------------------------
!include "MUI2.nsh"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY        ; Ask user for install folder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

;---------------------------------------------------------
; Installer section
;---------------------------------------------------------
Section "Install" SEC_INSTALL

    SetOutPath "$INSTDIR"

    ; --- 1) Copy your zip to a temp folder and unzip it ---
    File "fOptics.zip"                         ; bundle zip inside the installer

    nsisunz::Unzip "$INSTDIR\fOptics.zip" "$INSTDIR"
    Pop $0                                     ; get return value
    StrCmp $0 "success" +2                     ; skip error message if OK
        MessageBox MB_OK "Unzip failed: $0"

    Delete "$INSTDIR\fOptics.zip"              ; clean up zip after extraction

    ; --- 2) Run the .exe inside the unzipped folder ---
    ; Adjust the path to match where your .exe is after unzip
    ExecWait '"$INSTDIR\fOptics\Software\bin\vc_redist.x64.exe" /install /quiet /norestart'
	
	; --- 3) Create Desktop shortcut ---
    CreateShortCut "$DESKTOP\foptics.lnk"        \
                   "$INSTDIR\fOptics\Software\bin\fOptics.exe" \
                   ""                          \
                   "$INSTDIR\fOptics\Software\bin\fOptics.exe" \
                   0
				   
				   ; Bundle the icon file inside the installer
	File "icone_qkP_icon.ico"

	; Create desktop shortcut with icon
	CreateShortCut "$DESKTOP\foptics.lnk"         \
				   "$INSTDIR\fOptics\Software\bin\fOptics.exe" \
				   ""                            \
				   "$INSTDIR\icone_qkP_icon.ico"          \
				   0

    ; --- Write uninstaller ---
    WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;---------------------------------------------------------
; Uninstaller section
;---------------------------------------------------------
Section "Uninstall"

    ; Remove desktop shortcut
    Delete "$DESKTOP\foptics.lnk"

    ; Remove install directory
    RMDir /r "$INSTDIR"

SectionEnd