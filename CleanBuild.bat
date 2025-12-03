@echo off
REM CleanBuild.bat - clean then build the project (place next to build.bat)
REM Usage:
REM   CleanBuild.bat [Target] [Platform] [Configuration] [PathToUProject]
REM Examples:
REM   CleanBuild.bat
REM   CleanBuild.bat LyraEditor Win64 Development "C:\Projects\SnowWorld\SnowWorld.uproject"

setlocal EnableExtensions EnableDelayedExpansion

:: Determine script directory (trailing backslash included)
set "SCRIPT_DIR=%~dp0"

:: Engine root - can be overridden by setting UE_ENGINE_ROOT environment variable
if defined UE_ENGINE_ROOT (
    set "ENGINE_ROOT=%UE_ENGINE_ROOT%"
) else (
    set "ENGINE_ROOT=C:\Epic\UE_5.7"
)

:: Default build parameters (match existing build.bat defaults)
set "TARGET=LyraEditor"
set "PLATFORM=Win64"
set "CONFIG=Development"
set "UPROJECT=%SCRIPT_DIR%SnowWorld.uproject"

:: Allow overrides from command-line args
if not "%~1"=="" set "TARGET=%~1"
if not "%~2"=="" set "PLATFORM=%~2"
if not "%~3"=="" set "CONFIG=%~3"
if not "%~4"=="" set "UPROJECT=%~4"

:: Quick check for engine clean script
if not exist "%ENGINE_ROOT%\Engine\Build\BatchFiles\Clean.bat" (
    echo ERROR: Engine Clean script not found at "%ENGINE_ROOT%\Engine\Build\BatchFiles\Clean.bat"
    echo Please set the environment variable UE_ENGINE_ROOT to your Unreal Engine root
    exit /b 2
)

echo.
echo ------------------------------------------------------------
echo CleanBuild - Clean then Build
echo Target:     %TARGET%
echo Platform:   %PLATFORM%
echo Config:     %CONFIG%
echo UProject:   %UPROJECT%
echo EngineRoot: %ENGINE_ROOT%
echo ------------------------------------------------------------
echo.

:: Run Clean
echo Running Clean...
"%ENGINE_ROOT%\Engine\Build\BatchFiles\Clean.bat" %TARGET% %PLATFORM% %CONFIG% "%UPROJECT%" -waitmutex
if %ERRORLEVEL% NEQ 0 (
    echo Clean failed with exit code %ERRORLEVEL%.
    exit /b %ERRORLEVEL%
)

:: Run Build
echo Running Build...
"%ENGINE_ROOT%\Engine\Build\BatchFiles\Build.bat" %TARGET% %PLATFORM% %CONFIG% "%UPROJECT%" -waitmutex
if %ERRORLEVEL% NEQ 0 (
    echo Build failed with exit code %ERRORLEVEL%.
    exit /b %ERRORLEVEL%
)

echo.
echo CleanBuild completed successfully.
endlocal
exit /b 0
