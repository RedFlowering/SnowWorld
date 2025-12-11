@echo off
echo ========================================
echo   Starting SnowWorld - Client
echo ========================================
echo.

set ENGINE_PATH=C:\Epic\UE_5.7
set PROJECT_PATH=%~dp0SnowWorld.uproject

REM Server IP address - change this to your server's IP
set SERVER_IP=127.0.0.1

echo Engine Path: %ENGINE_PATH%
echo Project Path: %PROJECT_PATH%
echo Connecting to: %SERVER_IP%
echo.

"%ENGINE_PATH%\Engine\Binaries\Win64\UnrealEditor.exe" "%PROJECT_PATH%" %SERVER_IP% -game -log -windowed -ResX=1280 -ResY=720

pause
