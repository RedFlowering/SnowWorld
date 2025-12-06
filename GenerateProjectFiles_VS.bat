@echo off
REM UnrealVersionSelector.exe를 사용해 .sln 파일 생성
set UPROJECT_PATH=%~dp0SnowWorld.uproject
set UE_SELECTOR="C:\Program Files (x86)\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe"
if exist %UE_SELECTOR% (
    %UE_SELECTOR% /projectfiles "%UPROJECT_PATH%"
    echo Visual Studio project files generated successfully!
) else (
    echo UnrealVersionSelector.exe not found. Please check the path.
)
 