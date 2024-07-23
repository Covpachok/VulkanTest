@echo off
setlocal

rem Initialize variables
set BUILD_TYPE=Debug
set RUN_PROGRAM=false

rem Check parameters
:process_params
if "%~1"=="" goto done_params
if /I "%~1"=="-Debug" set BUILD_TYPE=Debug
if /I "%~1"=="-Release" set BUILD_TYPE=Release
if /I "%~1"=="-Run" set RUN_PROGRAM=true
shift
goto process_params

:done_params

rem Set directories based on build type
if /I "%BUILD_TYPE%"=="Debug" (
    set BUILD_DIR=build_debug
) else if /I "%BUILD_TYPE%"=="Release" (
    set BUILD_DIR=build_release
)

call py update_source_files.py

rem Run cmake commands
cmake -G Ninja -B./%BUILD_DIR%/ -S. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -D CMAKE_CXX_COMPILER=clang++ CMAKE_C_COMPILER=clang

rem Run the .exe file if -Run parameter is specified
if "%RUN_PROGRAM%"=="true" (
    cmake --build ./%BUILD_DIR%/ && .\%BUILD_DIR%\Vulkan.exe
) else (
    cmake --build ./%BUILD_DIR%/ 
)

echo %ERRORLEVEL%

endlocal

