@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

set "PORT="
set "WATCH=0"
set "MS_PORT=0"
set "MS_EXPORT=0"
set "MS_BUILD=0"
set "MS_FLASH=0"
set "FLASH_RC=0"

:parse
if "%~1"=="" goto parsed
if /I "%~1"=="--help" goto help
if /I "%~1"=="-h" goto help
if /I "%~1"=="/?" goto help
if /I "%~1"=="--watch" (
    set "WATCH=1"
    shift
    goto parse
)
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
echo   flash.bat --watch      same, then open the serial console
echo   flash.bat --port COM4  use this port instead of the Espressif USB device
echo   flash.bat --help
echo.
echo --watch opens an ESPET window with the log. Close that window to stop.
echo Ctrl+C in that window also stops. Do not use Ctrl+] in this terminal.
echo.
echo ESP-IDF is %%USERPROFILE%%\esp\esp-idf, or IDF_PATH if that is already set.
echo The simulator is sim.bat. This script does not use board-sim.
echo After a run, prints wall times for port detect, export, build, and flash.
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

call :now_ms T_ALL

if not defined PORT (
    call :now_ms T0
    for /f "usebackq delims=" %%p in (`powershell -NoProfile -Command "$p = @(Get-CimInstance Win32_PnPEntity | Where-Object { $_.DeviceID -match 'VID_303A' -and $_.Name -match '\(COM\d+\)' }); if ($p.Count -eq 0) { exit 2 }; if ($p.Count -gt 1) { $p | ForEach-Object { [Console]::Error.WriteLine($_.Name) }; exit 3 }; if ($p[0].Name -match '\((COM\d+)\)') { $Matches[1] }"`) do set "PORT=%%p"
    call :elapsed_ms T0 MS_PORT
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
if "!WATCH!"=="1" (
    echo Flashing firmware on !PORT!, then opening the serial console
) else (
    echo Flashing firmware on !PORT!
)

call :now_ms T0
call "%IDF_PATH%\export.bat"
if errorlevel 1 (
    echo export.bat failed
    exit /b 1
)
call :elapsed_ms T0 MS_EXPORT

REM Ninja %e = wall seconds for the current edge (no idf.py --time flag).
if not defined NINJA_STATUS set "NINJA_STATUS=[%%f/%%t %%es] "

call :now_ms T0
idf.py -C firmware build
set "FLASH_RC=!ERRORLEVEL!"
call :elapsed_ms T0 MS_BUILD
if not "!FLASH_RC!"=="0" goto timed_done

call :now_ms T0
idf.py -C firmware -p !PORT! flash
set "FLASH_RC=!ERRORLEVEL!"
call :elapsed_ms T0 MS_FLASH

:timed_done
call :elapsed_ms T_ALL MS_TOTAL
call :print_timings
if not "!WATCH!"=="1" goto flash_exit
if not "!FLASH_RC!"=="0" goto flash_exit

echo.
echo Serial log is in the ESPET window. Close that window to stop.
set "PY=!IDF_PYTHON_ENV_PATH!\Scripts\python.exe"
if not exist "!PY!" set "PY=python"
start "ESPET" /wait "!PY!" "%IDF_PATH%\tools\idf.py" -C firmware -p !PORT! monitor
set "FLASH_RC=0"

:flash_exit
exit /b !FLASH_RC!

:now_ms
for /f %%i in ('powershell -NoProfile -Command "[int64]([DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds())"') do set "%~1=%%i"
exit /b 0

:elapsed_ms
for /f %%i in ('powershell -NoProfile -Command "[int64]([DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds()) - [int64]'!%~1!'"') do set "%~2=%%i"
exit /b 0

:fmt_ms
set "OUT=%~2"
for /f "delims=" %%f in ('powershell -NoProfile -Command "$ms=[int64]'%~1'; if ($ms -ge 60000) { '{0}m {1:N1}s' -f [int]($ms/60000), (($ms%%60000)/1000.0) } else { '{0:N1}s' -f ($ms/1000.0) }"') do set "%OUT%=%%f"
exit /b 0

:print_timings
echo.
echo === flash.bat timings ===
if not defined MS_TOTAL set "MS_TOTAL=0"
call :fmt_ms !MS_PORT! F_PORT
call :fmt_ms !MS_EXPORT! F_EXPORT
call :fmt_ms !MS_BUILD! F_BUILD
call :fmt_ms !MS_FLASH! F_FLASH
call :fmt_ms !MS_TOTAL! F_TOTAL
echo   port detect : !F_PORT!
echo   idf export  : !F_EXPORT!
echo   idf build   : !F_BUILD!
echo   idf flash   : !F_FLASH!
echo   total       : !F_TOTAL!
exit /b 0
