@echo off
echo ========================================
echo   Packaging SnowWorld - Shipping
echo ========================================
echo.

set ENGINE_PATH=C:\Epic\UE_5.7
set PROJECT_PATH=%~dp0SnowWorld.uproject
set OUTPUT_PATH=%~dp0Packaged\Shipping

echo Engine Path: %ENGINE_PATH%
echo Project Path: %PROJECT_PATH%
echo Output Path: %OUTPUT_PATH%
echo.

echo Starting packaging process...
echo.

"%ENGINE_PATH%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
    -project="%PROJECT_PATH%" ^
    -noP4 ^
    -platform=Win64 ^
    -clientconfig=Shipping ^
    -serverconfig=Shipping ^
    -cook ^
    -allmaps ^
    -build ^
    -stage ^
    -pak ^
    -archive ^
    -archivedirectory="%OUTPUT_PATH%" ^
    -utf8output

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo   Packaging Failed!
    echo ========================================
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ========================================
echo   Packaging Successful!
echo   Output: %OUTPUT_PATH%
echo ========================================
pause
