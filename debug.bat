@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

if /I "%~1"=="--help" goto help
if /I "%~1"=="-h" goto help
if /I "%~1"=="/?" goto help
if not "%~1"=="" (
    echo Unknown flag: %~1
    echo.
    goto help_err
)

call :find_sln
if defined SLN goto find_devenv

if exist "build-sim\CMakeCache.txt" (
    echo build-sim has no Visual Studio solution. Wiping and reconfiguring...
    call sim.bat --clean --debug --no-run
) else (
    echo Configuring board-sim for Visual Studio...
    call sim.bat --debug --no-run
)
if errorlevel 1 exit /b 1

call :find_sln
if not defined SLN (
    echo Could not find build-sim\espet-board-sim.slnx or .sln
    echo CMake must use the Visual Studio generator, not Ninja.
    exit /b 1
)

:find_devenv
set "DEVENV="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!VSWHERE!" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "!VSWHERE!" (
    for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.CoreIde -property productPath`) do (
        set "DEVENV=%%i"
    )
)
if not defined DEVENV if exist "f:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\devenv.exe" (
    set "DEVENV=f:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\devenv.exe"
)
if not exist "!DEVENV!" (
    echo Could not find Visual Studio IDE ^(devenv.exe^).
    echo Build Tools can compile sim.bat but cannot debug.
    echo Install Community / Professional with the IDE, then retry.
    exit /b 1
)

echo Opening !DEVENV!
echo Solution: !SLN!
echo In VS: Debug / x64, startup project espet-board-sim, then F5.
echo Close the sim window to stop. Do not also run sim.bat.
start "" "!DEVENV!" "%cd%\!SLN!"
exit /b 0

:find_sln
set "SLN="
if exist "build-sim\espet-board-sim.slnx" set "SLN=build-sim\espet-board-sim.slnx"
if not defined SLN if exist "build-sim\espet-board-sim.sln" set "SLN=build-sim\espet-board-sim.sln"
exit /b 0

:help
echo ESPET board simulator — open in Visual Studio
echo.
echo   debug.bat               configure Debug if needed, then open the solution
echo   debug.bat --help        this text
echo.
echo Uses the IDE ^(devenv^), not Build Tools. In VS: Debug / x64 / F5.
echo Do not run sim.bat at the same time — one window, one process.
echo.
echo Firmware lives in firmware\  (no SDL^). The window is board-sim\ only.
exit /b 0

:help_err
call :help
exit /b 1
