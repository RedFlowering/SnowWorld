@echo off
echo Building SnowWorldEditor...
"C:\Epic\UE_5.7\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" LyraEditor Win64 Development -Project="%~dp0SnowWorld.uproject" -WaitMutex
if %ERRORLEVEL% NEQ 0 (
    echo Build Failed!
    exit /b %ERRORLEVEL%
)
echo Build Successful!
