@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

set "DO_CLEAN=0"
set "DO_BUILD=1"
set "DO_RUN=1"
set "CONFIG=Release"

:parse
if "%~1"=="" goto parsed
if /I "%~1"=="--help" goto help
if /I "%~1"=="-h" goto help
if /I "%~1"=="/?" goto help
if /I "%~1"=="--clean" (
    set "DO_CLEAN=1"
    shift
    goto parse
)
if /I "%~1"=="--debug" (
    set "CONFIG=Debug"
    shift
    goto parse
)
if /I "%~1"=="--no-run" (
    set "DO_RUN=0"
    shift
    goto parse
)
if /I "%~1"=="--run-only" (
    set "DO_BUILD=0"
    set "DO_RUN=1"
    shift
    goto parse
)
echo Unknown flag: %~1
echo.
goto help_err

:help
echo ESPET board simulator
echo.
echo   sim.bat                 configure (if needed) + build Release + run
echo   sim.bat --clean         delete build-sim, then the same
echo   sim.bat --debug         Debug instead of Release
echo   sim.bat --no-run        build only
echo   sim.bat --run-only      skip build; start the last exe
echo   sim.bat --help          this text
echo   debug.bat               open the solution in Visual Studio ^(F5^)
echo.
echo Optional env: set BOARD_SIM_SPI_HZ=40000000
echo   (fake SPI duration on draw_bitmap; unset = off)
echo.
echo Firmware lives in firmware\  (no SDL^). The window is board-sim\ only.
exit /b 0

:help_err
call :help
exit /b 1

:parsed

if "!DO_BUILD!"=="1" goto need_vs
goto find_exe

:need_vs
REM Prefer an install that has the MSVC toolset (Build Tools / C++ workload).
REM The Community copy on F: may ship vcvars64.bat without vcvarsall.bat.
set "VCVARS="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!VSWHERE!" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "!VSWHERE!" (
    for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        if exist "%%i\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=%%i\VC\Auxiliary\Build\vcvars64.bat"
    )
)
if not defined VCVARS if exist "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)
if not defined VCVARS if exist "f:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VCVARS=f:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
)
if not exist "!VCVARS!" (
    echo Could not find a Visual Studio C++ toolchain ^(vcvarsall.bat^).
    echo Install "Desktop development with C++" or VS Build Tools, then retry.
    exit /b 1
)

:have_vcvars
echo Using VS: !VCVARS!
call "!VCVARS!" >nul
if errorlevel 1 (
    echo vcvars64.bat failed
    exit /b 1
)
if defined VSINSTALLDIR (
    set "CMAKE_BIN=!VSINSTALLDIR!Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
    set "NINJA_BIN=!VSINSTALLDIR!Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
    if exist "!CMAKE_BIN!\cmake.exe" set "PATH=!CMAKE_BIN!;!PATH!"
    if exist "!NINJA_BIN!\ninja.exe" set "PATH=!NINJA_BIN!;!PATH!"
)
where cmake >nul 2>&1
if errorlevel 1 (
    echo cmake not found. Install the CMake tools workload in Visual Studio, or add cmake to PATH.
    exit /b 1
)

:cmake_ok
if "!DO_CLEAN!"=="1" (
    echo Cleaning build-sim
    if exist build-sim rmdir /s /q build-sim
)

if not exist build-sim\CMakeCache.txt (
    echo Configuring board-sim
    cmake -S board-sim -B build-sim -G "Visual Studio 18 2026" -A x64
    if errorlevel 1 (
        echo VS 18 2026 generator failed, trying Ninja...
        if exist build-sim rmdir /s /q build-sim
        cmake -S board-sim -B build-sim -G Ninja -DCMAKE_BUILD_TYPE=!CONFIG!
        if errorlevel 1 (
            echo CMake configure failed
            exit /b 1
        )
    )
)

echo Building !CONFIG!
cmake --build build-sim --config !CONFIG! --parallel
if errorlevel 1 (
    echo Build failed
    exit /b 1
)

:find_exe
set "EXE=build-sim\!CONFIG!\espet-board-sim.exe"
if not exist "!EXE!" set "EXE=build-sim\espet-board-sim.exe"
if not exist "!EXE!" (
    echo Could not find espet-board-sim.exe
    echo Looked at build-sim\!CONFIG!\ and build-sim\
    echo Build first: sim.bat --no-run
    exit /b 1
)

if "!DO_RUN!"=="0" (
    echo Built: !EXE!
    exit /b 0
)

echo Running !EXE!
echo Drag in the window to tilt the fake IMU. Close the window to quit.
"!EXE!"
exit /b %ERRORLEVEL%
