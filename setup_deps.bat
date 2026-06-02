@echo off
REM =============================================================
REM  AstroDefender - Dependency Setup Script
REM  Run this once before opening the Visual Studio solution.
REM  Requires: PowerShell, internet access
REM =============================================================

echo.
echo  AstroDefender - Dependency Setup
echo  =================================
echo.

SET DEPS_DIR=%~dp0deps

IF NOT EXIST "%DEPS_DIR%" mkdir "%DEPS_DIR%"

echo  Downloading SDL2 development libraries...
echo.

REM --- SDL2 ---
SET SDL2_URL=https://github.com/libsdl-org/SDL/releases/download/release-2.30.3/SDL2-devel-2.30.3-VC.zip
SET SDL2_ZIP=%DEPS_DIR%\SDL2.zip
SET SDL2_DIR=%DEPS_DIR%\SDL2

IF NOT EXIST "%SDL2_DIR%\include\SDL.h" (
    echo  [1/3] Downloading SDL2...
    powershell -Command "Invoke-WebRequest -Uri '%SDL2_URL%' -OutFile '%SDL2_ZIP%'"
    echo  [1/3] Extracting SDL2...
    powershell -Command "Expand-Archive -Path '%SDL2_ZIP%' -DestinationPath '%DEPS_DIR%\SDL2_tmp' -Force"
    powershell -Command "Move-Item '%DEPS_DIR%\SDL2_tmp\SDL2-2.30.3' '%SDL2_DIR%' -Force; Remove-Item '%DEPS_DIR%\SDL2_tmp' -Recurse -Force"
    del "%SDL2_ZIP%"
    echo  [1/3] SDL2 ready.
) ELSE (
    echo  [1/3] SDL2 already present, skipping.
)

REM --- SDL2_ttf ---
SET TTF_URL=https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-devel-2.22.0-VC.zip
SET TTF_ZIP=%DEPS_DIR%\SDL2_ttf.zip
SET TTF_DIR=%DEPS_DIR%\SDL2_ttf

IF NOT EXIST "%TTF_DIR%\include\SDL_ttf.h" (
    echo  [2/3] Downloading SDL2_ttf...
    powershell -Command "Invoke-WebRequest -Uri '%TTF_URL%' -OutFile '%TTF_ZIP%'"
    echo  [2/3] Extracting SDL2_ttf...
    powershell -Command "Expand-Archive -Path '%TTF_ZIP%' -DestinationPath '%DEPS_DIR%\TTF_tmp' -Force"
    powershell -Command "Move-Item '%DEPS_DIR%\TTF_tmp\SDL2_ttf-2.22.0' '%TTF_DIR%' -Force; Remove-Item '%DEPS_DIR%\TTF_tmp' -Recurse -Force"
    del "%TTF_ZIP%"
    echo  [2/3] SDL2_ttf ready.
) ELSE (
    echo  [2/3] SDL2_ttf already present, skipping.
)

REM --- SDL2_mixer ---
SET MIX_URL=https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.8.0/SDL2_mixer-devel-2.8.0-VC.zip
SET MIX_ZIP=%DEPS_DIR%\SDL2_mixer.zip
SET MIX_DIR=%DEPS_DIR%\SDL2_mixer

IF NOT EXIST "%MIX_DIR%\include\SDL_mixer.h" (
    echo  [3/3] Downloading SDL2_mixer...
    powershell -Command "Invoke-WebRequest -Uri '%MIX_URL%' -OutFile '%MIX_ZIP%'"
    echo  [3/3] Extracting SDL2_mixer...
    powershell -Command "Expand-Archive -Path '%MIX_ZIP%' -DestinationPath '%DEPS_DIR%\MIX_tmp' -Force"
    powershell -Command "Move-Item '%DEPS_DIR%\MIX_tmp\SDL2_mixer-2.8.0' '%MIX_DIR%' -Force; Remove-Item '%DEPS_DIR%\MIX_tmp' -Recurse -Force"
    del "%MIX_ZIP%"
    echo  [3/3] SDL2_mixer ready.
) ELSE (
    echo  [3/3] SDL2_mixer already present, skipping.
)

echo.
echo  All dependencies are ready.
echo  You can now open AstroDefender.sln in Visual Studio.
echo.
pause
