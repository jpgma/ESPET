@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

set "PORT="

:parse
if "%~1"=="" goto parsed
if /I "%~1"=="--help" goto help
if /I "%~1"=="-h" goto help
if /I "%~1"=="/?" goto help
if /I "%~1"=="--port" (
    shift
    if "%~1"=="" (
        echo --port needs a COM port
        exit /b 1
    )
    set "PORT=%~1"
    shift
    goto parse
)
echo Unknown flag: %~1
echo.
goto help_err

:help
echo Flash the ESPET firmware on the Waveshare ESP32-S3-Touch-LCD-1.54.
echo.
echo   flash.bat              build and flash firmware\
echo   flash.bat --port COM4  use this port instead of the Espressif USB device
echo   flash.bat --help
echo.
echo ESP-IDF is %%USERPROFILE%%\esp\esp-idf, or IDF_PATH if that is already set.
echo The simulator is sim.bat. This script does not use board-sim.
exit /b 0

:help_err
call :help
exit /b 1

:parsed
if not exist "firmware\CMakeLists.txt" (
    echo firmware\CMakeLists.txt is missing.
    exit /b 1
)

if not defined IDF_PATH set "IDF_PATH=%USERPROFILE%\esp\esp-idf"
if not exist "%IDF_PATH%\export.bat" (
    echo ESP-IDF export.bat not found at %IDF_PATH%
    echo Install ESP-IDF 5.5 or set IDF_PATH.
    exit /b 1
)

if not defined PORT (
    for /f "usebackq delims=" %%p in (`powershell -NoProfile -Command "$p = @(Get-CimInstance Win32_PnPEntity | Where-Object { $_.DeviceID -match 'VID_303A' -and $_.Name -match '\(COM\d+\)' }); if ($p.Count -eq 0) { exit 2 }; if ($p.Count -gt 1) { $p | ForEach-Object { [Console]::Error.WriteLine($_.Name) }; exit 3 }; if ($p[0].Name -match '\((COM\d+)\)') { $Matches[1] }"`) do set "PORT=%%p"
    if not defined PORT (
        echo No Espressif USB serial port found. Plug in the board, or pass --port COMx.
        exit /b 1
    )
    if /I "!PORT!"=="COM" (
        echo More than one Espressif serial port. Pass --port COMx.
        exit /b 1
    )
)

echo Using ESP-IDF: %IDF_PATH%
echo Flashing firmware on !PORT!
call "%IDF_PATH%\export.bat"
if errorlevel 1 (
    echo export.bat failed
    exit /b 1
)
idf.py -C firmware -p !PORT! build flash
exit /b !ERRORLEVEL!
