@echo off
chcp 65001 >nul
call "D:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

echo.
echo === VS Environment Loaded ===
echo VSCMD_VER=%VSCMD_VER%
echo.

cmake -B Builds -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DKFR_ENABLE_MULTIARCH=OFF -DZL_JUCE_FORMATS="VST3" -DZL_JUCE_COPY_PLUGIN=FALSE.

if %errorlevel% neq 0 (
    echo.
    echo CMake configuration failed.
    pause
    exit /b %errorlevel%
)

echo.
echo Configuration successful. To build, run:
echo   cmake --build Builds --config Release
echo.
pause
