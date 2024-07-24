@echo off

set SHADER_COMPILER=F:\Programs\VulkanSDK\1.3.283.0\Bin\glslc.exe
set SHADER_DIR=src\shaders

set SHADER_BUILD_DIR=%BUILD_DIR%\shaders
mkdir %SHADER_BUILD_DIR%

echo ##COMPILING SHADERS

call %SHADER_COMPILER% -fshader-stage=vert %SHADER_DIR%\shader_vert.glsl -o %SHADER_BUILD_DIR%\shader_vert.spv
call %SHADER_COMPILER% -fshader-stage=frag %SHADER_DIR%\shader_frag.glsl -o %SHADER_BUILD_DIR%\shader_frag.spv

echo ##SHADER COMPILATION COMPLETED
