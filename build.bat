@echo off
setlocal

set BUILD_TYPE=Debug
set RUN_PROGRAM=false
set PROGRAM_NAME=Vulkan

:process_params
if "%~1"=="" goto done_params
if /I "%~1"=="-Debug" set BUILD_TYPE=Debug
if /I "%~1"=="-Release" set BUILD_TYPE=Release
if /I "%~1"=="-Run" set RUN_PROGRAM=true
shift
goto process_params

:done_params

if /I "%BUILD_TYPE%"=="Debug" (
    set BUILD_DIR=build_debug
) else if /I "%BUILD_TYPE%"=="Release" (
    set BUILD_DIR=build_release
)


echo:
call py update_source_files.py
echo:

echo:
call compile_shaders.bat -%BUILD_TYPE%
echo:

echo:
cmake -G Ninja -B./%BUILD_DIR%/ -S. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -D CMAKE_CXX_COMPILER=clang++ CMAKE_C_COMPILER=clang
echo:

if "%RUN_PROGRAM%"=="true" (
    echo:
    cmake --build ./%BUILD_DIR%/ 

    if ERRORLEVEL 1 (
        exit /b %ERRORLEVEL%
    )
    echo:

    echo:
    set PREV_DIR=%cd%
    cd %BUILD_DIR%
    .\%PROGRAM_NAME%.exe 
    cd %PREV_DIR%
    echo Program exited with code: %ERRORLEVEL%

) else (
    echo:
    cmake --build ./%BUILD_DIR%/ 
)

endlocal

