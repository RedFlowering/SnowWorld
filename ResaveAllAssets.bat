@echo off
REM =========================================================
REM ResaveAllAssets.bat - Unreal Engine 5.7 Project Resave
REM =========================================================
REM 모든 프로젝트 애셋을 현재 엔진 버전으로 리세이브합니다.
REM 엔진 업그레이드 후 또는 "empty engine version" 경고 해결에 사용하세요.
REM =========================================================

set UE_PATH=C:\Epic\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe
set PROJECT_PATH=C:\Projects\SnowWorld\SnowWorld.uproject

echo =========================================================
echo Resaving all assets in SnowWorld project...
echo This may take a while. Please wait...
echo =========================================================

"%UE_PATH%" "%PROJECT_PATH%" -run=ResavePackages -AutoCheckOut -OnlySaveDirtyPackages=false -IgnoreChangelist

echo.
echo =========================================================
echo Resave complete!
echo =========================================================
pause
