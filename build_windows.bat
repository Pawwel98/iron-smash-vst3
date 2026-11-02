@echo off
setlocal
echo ============================================================
echo   Building IRON SMASH - Rock/Metal Drum Bus Enhancer (VST3)
echo ============================================================

where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake is not installed or not in PATH.
    echo Install CMake from https://cmake.org/download/ and Visual Studio Build Tools.
    pause
    exit /b 1
)

cmake -B build -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

cmake --build build --config Release --parallel
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

echo.
echo [SUCCESS] VST3 plugin built successfully!
echo Location: build\RockDrumEnhancer_artefacts\Release\VST3\Iron Smash Drum Enhancer.vst3
echo Copy the .vst3 folder into: C:\Program Files\Common Files\VST3\
pause
