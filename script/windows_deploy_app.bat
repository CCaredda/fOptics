@echo off
setlocal enableextensions

REM TODO: create fOptics python env

set "APP_NAME=fOptics"
set "PROJECT_DIR=%~dp0\.."


set "BUILD_DIR=%PROJECT_DIR%\main\release"
set "PATH_PRO=%PROJECT_DIR%\main\Process_multi_thread_RT.pro"


REM Ask user to select a folder
for /f "usebackq delims=" %%i in (`powershell -NoProfile -Command ^
    "Add-Type -AssemblyName System.Windows.Forms; " ^
    "$f = New-Object System.Windows.Forms.FolderBrowserDialog; " ^
    "$f.Description='Select a folder'; " ^
    "$f.ShowNewFolderButton=$true; " ^
    "if($f.ShowDialog() -eq 'OK'){Write-Output $f.SelectedPath}"`) do (
    set "FINAL_DIR=%%i"
)

set "FINAL_DIR=%FINAL_DIR%\%APP_NAME%"

set "DEPLOY_DIR=%FINAL_DIR%\Software\bin"

set "QT_BIN=C:\Qt\6.7.3\msvc2022_64\bin"


set "OPENCV_BIN=C:\opencv\install\x64\vc17\bin"
set "FFTW_BIN=C:\fftw\lib"




REM Create soft dir
mkdir "%FINAL_DIR%"
mkdir "%FINAL_DIR%\Software"


REM Copy doc
mkdir "%FINAL_DIR%\Doc"
xcopy "%PROJECT_DIR%\doc_src\usage_doc.odt" "%FINAL_DIR%\Doc\" /Y
xcopy "%PROJECT_DIR%\doc_src\Installation_doc.odt" "%FINAL_DIR%\Doc\" /Y

REM Copy Python files
mkdir "%FINAL_DIR%\Python_files"

xcopy "%PROJECT_DIR%\Python\Compute_results_and_metrics.ipynb" "%FINAL_DIR%\Python_files\" /Y
xcopy "%PROJECT_DIR%\Python\utils.py" "%FINAL_DIR%\Python_files\" /Y
xcopy "%PROJECT_DIR%\Python\create_Acquisition_info_file.py" "%FINAL_DIR%\Python_files\" /Y

REM Copy script for creating venv
xcopy "%PROJECT_DIR%\script\windows_create_venv.bat" "%FINAL_DIR%\" /Y

REM Compile Qt Project
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

cd "%FINAL_DIR%\Software"
%QT_BIN%\qmake "%PATH_PRO%" CONFIG+=release
nmake


rem --- delete all files in %FINAL_DIR%\Software ---
for %%F in ("%FINAL_DIR%\Software\*") do (
    if /I not "%%~nxF"=="release" (
        del /q "%%F" 2>nul
    )
)

for /d %%D in ("%FINAL_DIR%\Software\*") do (
    if /I not "%%~nxD"=="release" (
        rmdir /s /q "%%D"
    )
)


REM Rename release folder to bin
ren "%FINAL_DIR%\Software\release" "bin"


REM Get all DLLs
cd "%FINAL_DIR%\Software\bin"
"%QT_BIN%\windeployqt.exe" --release --compiler-runtime --no-translations "%DEPLOY_DIR%\Process_multi_thread_RT.exe"
if errorlevel 1 goto :error


REM Remove tempory files
del /q "%DEPLOY_DIR%\moc_*"
del /q "%DEPLOY_DIR%\*.obj"



REM Copy share folder
robocopy "%PROJECT_DIR%\main\share" "%FINAL_DIR%\Software\share" /E


REM Copy OpenCV dll
xcopy "%OPENCV_BIN%\opencv_world*.dll" "%DEPLOY_DIR%\" /Y
xcopy "%OPENCV_BIN%\opencv_videoio_*.dll" "%DEPLOY_DIR%\" /Y

REM Copy FFTW dll
xcopy "%FFTW_BIN%\fftw3*.lib" "%DEPLOY_DIR%\" /Y

REM Create shortcut with icon
set "TARGET=%DEPLOY_DIR%\Process_multi_thread_RT.exe"
set "SHORTCUT=%FINAL_DIR%\%APP_NAME%.lnk"
set "ICON=%~dp0icone_qkP_icon.ico"

set "TMPVBS=%TEMP%\mkshortcut_%RANDOM%.vbs"

rem Write VBS line-by-line; escape the closing ) with ^)
> "%TMPVBS%"  echo Set oWS = CreateObject("WScript.Shell"^)
>> "%TMPVBS%" echo Set oLink = oWS.CreateShortcut("%SHORTCUT%")
>> "%TMPVBS%" echo oLink.TargetPath = "%TARGET%"
>> "%TMPVBS%" echo oLink.WorkingDirectory = "%FINAL_DIR%"
>> "%TMPVBS%" echo oLink.IconLocation = "%ICON%,0"
>> "%TMPVBS%" echo oLink.Save

if not exist "%TMPVBS%" (
  echo Failed to create VBS: "%TMPVBS%"
  exit /b 1
)

cscript //nologo "%TMPVBS%"
del "%TMPVBS%"



endlocal
