@echo off
setlocal
cd /d "%~dp0"
echo ESPET desk. Release the port from the tray before flash.bat.
dotnet run --project studio
exit /b %ERRORLEVEL%
