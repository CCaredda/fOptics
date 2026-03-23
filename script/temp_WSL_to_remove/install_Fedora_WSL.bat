@echo off
setlocal

mkdir c:\wsl\distros
cd c:\wsl\distros

REM Set the URL and output file name
set "URL=https://github.com/WhitewaterFoundry/Fedora-Remix-for-WSL/archive/refs/tags/41.0.0.tar.gz?raw=true"
set "OUTPUT=41.0.0.tar.gz"

REM Download the file using PowerShell
echo Downloading %OUTPUT%...
powershell -Command "Invoke-WebRequest -Uri '%URL%' -OutFile '%OUTPUT%'"

REM Check if the file was downloaded
if exist "%OUTPUT%" (
    echo Download completed.
    
    wsl --import fedora41 c:\wsl\distros Fedora-Remix-for-WSL-41.0.0.tar.gz
)
