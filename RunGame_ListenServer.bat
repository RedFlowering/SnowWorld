@echo off
echo ========================================
echo   Starting SnowWorld - Listen Server
echo ========================================
echo.

set ENGINE_PATH=C:\Epic\UE_5.7
set PROJECT_PATH=%~dp0SnowWorld.uproject

echo Engine Path: %ENGINE_PATH%
echo Project Path: %PROJECT_PATH%
echo.

"%ENGINE_PATH%\Engine\Binaries\Win64\UnrealEditor.exe" "%PROJECT_PATH%" ?listen -game -log -windowed -ResX=1280 -ResY=720

pause
