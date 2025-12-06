@echo off
REM VSCode용 .code-workspace 파일 생성
set WORKSPACE_FILE=%~dp0SnowWorld.code-workspace
echo { > "%WORKSPACE_FILE%"
echo   "folders": [ >> "%WORKSPACE_FILE%"
echo     { "path": "." } >> "%WORKSPACE_FILE%"
echo   ], >> "%WORKSPACE_FILE%"
echo   "settings": {} >> "%WORKSPACE_FILE%"
echo } >> "%WORKSPACE_FILE%"
echo VSCode workspace file generated successfully!