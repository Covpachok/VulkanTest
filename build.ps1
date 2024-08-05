param (
    [switch]$Debug,
    [switch]$Release,
    [switch]$Run,
    [switch]$Gen
)

$BUILD_TYPE = "Debug"
$RUN_PROGRAM = $false
$PROGRAM_NAME = "Vulkan"
$GENERATE_CMAKE = $false

if ($Debug) {
    $BUILD_TYPE = "Debug"
}

if ($Release) {
    $BUILD_TYPE = "Release"
}

if ($Run) {
    $RUN_PROGRAM = $true
}

if ($Gen) {
    $GENERATE_CMAKE = $true
}

if ($BUILD_TYPE -eq "Debug") {
    $BUILD_DIR = "build_debug"
} elseif ($BUILD_TYPE -eq "Release") {
    $BUILD_DIR = "build_release"
}

Write-Output ""

& python update_source_files.py

Write-Output ""

& .\compile_shaders.bat -$BUILD_TYPE

Write-Output ""

if ($GENERATE_CMAKE) {
    Write-Output ""
    cmake -G Ninja -B "./$BUILD_DIR/" -S . -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DDO_SANITIZE=true
    Write-Output ""
}

if ($RUN_PROGRAM) {
    Write-Output ""
    cmake --build "./$BUILD_DIR/"

    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    Write-Output ""
    $PREV_DIR = Get-Location
    Set-Location $BUILD_DIR
    & ".\$PROGRAM_NAME.exe"
    Write-Output "Program exited with code: $LASTEXITCODE"
    Set-Location $PREV_DIR
} else {
    Write-Output ""
    cmake --build "./$BUILD_DIR/"
}

