@echo off
echo ========================================
echo   Starting SnowWorld - Server + Clients
echo ========================================
echo.

set ENGINE_PATH=C:\Epic\UE_5.7
set PROJECT_PATH=%~dp0SnowWorld.uproject

REM Default map - change this to your desired map
set MAP=/Game/Maps/L_Harmonia

REM Number of additional clients (besides the listen server)
set NUM_CLIENTS=1

echo Engine Path: %ENGINE_PATH%
echo Project Path: %PROJECT_PATH%
echo Additional Clients: %NUM_CLIENTS%
echo.

echo Starting Listen Server...
start "" "%ENGINE_PATH%\Engine\Binaries\Win64\UnrealEditor.exe" "%PROJECT_PATH%" ?listen -game -log -windowed -ResX=1280 -ResY=720 -WinX=0 -WinY=50

REM Wait for server to initialize
timeout /t 5 /nobreak

echo Starting Client(s)...
for /L %%i in (1,1,%NUM_CLIENTS%) do (
    echo   Starting Client %%i...
    start "" "%ENGINE_PATH%\Engine\Binaries\Win64\UnrealEditor.exe" "%PROJECT_PATH%" 127.0.0.1 -game -log -windowed -ResX=1280 -ResY=720 -WinX=1300 -WinY=50
    timeout /t 2 /nobreak
)

echo.
echo ========================================
echo   All instances started!
echo ========================================
pause
