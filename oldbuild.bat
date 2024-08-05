@echo off
setlocal

set BUILD_TYPE=Debug
set RUN_PROGRAM=false
set PROGRAM_NAME=Vulkan
set GENERATE_CMAKE=false

:process_params
if "%~1"=="" goto done_params
if /I "%~1"=="-Debug" set BUILD_TYPE=Debug
if /I "%~1"=="-Release" set BUILD_TYPE=Release
if /I "%~1"=="-Run" set RUN_PROGRAM=true
if /I "%~1"=="-Gen" set GENERATE_CMAKE=true
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

if /I "%GENERATE_CMAKE%"=="true" (
    echo:
    cmake -G Ninja -B./%BUILD_DIR%/ -S. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -D CMAKE_CXX_COMPILER=clang++ -D CMAKE_C_COMPILER=clang -D DO_SANITIZE=true
    echo:
)

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
    echo Program exited with code: %ERRORLEVEL%
    cd %PREV_DIR%

) else (
    echo:
    cmake --build ./%BUILD_DIR%/ 
)

endlocal

