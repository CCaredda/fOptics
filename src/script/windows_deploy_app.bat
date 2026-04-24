@echo off
setlocal enableextensions

REM TODO: create fOptics python env

set "APP_NAME=fOptics"
for %%i in ("%~dp0..") do set "PROJECT_DIR=%%~fi"
set "MY_DIR=%~dp0"

set "BUILD_DIR=%PROJECT_DIR%\main\release"
set "PATH_PRO=%PROJECT_DIR%\main\fOptics.pro"


set "FINAL_DIR=%MY_DIR%%APP_NAME%"

set "DEPLOY_DIR=%FINAL_DIR%\Software\bin"

set "QT_BIN=C:\Qt\6.11.0\msvc2022_64\bin"


set "OPENCV_BIN=C:\opencv\install\x64\vc17\bin"
set "FFTW_BIN=C:\fftw\lib"
set "FFMPEG_BIN=C:\vcpkg\installed\x64-windows\bin"






REM Create soft dir
mkdir "%FINAL_DIR%"
mkdir "%FINAL_DIR%\Software"


REM Copy doc
mkdir "%FINAL_DIR%\Doc"
xcopy "%PROJECT_DIR%\Doc\usage_doc.pdf" "%FINAL_DIR%\Doc\" /Y
xcopy "%PROJECT_DIR%\Doc\Acquisition_infos.txt" "%FINAL_DIR%\Doc\" /Y



REM Compile Qt Project
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64

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
"%QT_BIN%\windeployqt.exe" --release --compiler-runtime --no-translations "%DEPLOY_DIR%\fOptics.exe"
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

REM Copy ffmpeg dll
xcopy "%FFMPEG_BIN%\*.dll" "%DEPLOY_DIR%\" /Y


xcopy "C:\Windows\System32\vcomp140.dll" "%DEPLOY_DIR%\" /Y


REM ── Configuration ─────────────────────────────────────────────────────
set "FOLDER_TO_ZIP=%FINAL_DIR%"
set "ZIP_OUTPUT=%~dp0fOptics.zip"
set "NSI_FILE=%~dp0installer.nsi"
set "MAKENSIS=C:\Program Files (x86)\NSIS\makensis.exe"

REM Zip the folder using PowerShell ────────────────────────────
echo Zipping folder...

REM Delete old zip if it exists
if exist "%ZIP_OUTPUT%" del /F /Q "%ZIP_OUTPUT%"

powershell -NoProfile -Command "Compress-Archive -Path '%FOLDER_TO_ZIP%' -DestinationPath '%ZIP_OUTPUT%' -CompressionLevel Optimal"

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Zip failed.
    pause
    exit /b 1
)
echo Zip created: %ZIP_OUTPUT%
del /F /Q "%FOLDER_TO_ZIP%"

REM ── Step 3: Compile the .nsi file ─────────────────────────────────────
echo Compiling NSIS installer...

"%MAKENSIS%" "%NSI_FILE%"
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: NSIS compilation failed.
    pause
    exit /b 1
)

echo Done! Installer created successfully.
del /f "%ZIP_OUTPUT%"

endlocal
