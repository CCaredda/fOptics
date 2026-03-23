@echo off
setlocal

rem Check if %USERPROFILE%\.venv exists
if not exist "%USERPROFILE%\.venv" (
    mkdir "%USERPROFILE%\.venv"
)

set "path_venv=%USERPROFILE%\.venv\Optics_venv"
echo %path_venv%

rem Remove venv if it exists
if exist "%path_venv%" (
    rmdir /s /q "%path_venv%"
)


rem Create venv
python -m venv "%path_venv%"

rem Activate venv
call "%path_venv%\Scripts\activate.bat"

rem Upgrade pip
python -m pip install --upgrade pip

rem Install requirements
pip install -r requirements.txt

endlocal

