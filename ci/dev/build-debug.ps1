param(
    [string]$ConfigurePreset = "tests",
    [string]$BuildPreset = "debug-tests",
    [switch]$RunTests
)

if ([string]::IsNullOrWhiteSpace($env:VCPKG_ROOT)) {
    throw "VCPKG_ROOT is not set. Install vcpkg once to a local path and then set the user environment variable VCPKG_ROOT to that location."
}

$toolchainFile = Join-Path $env:VCPKG_ROOT "scripts\buildsystems\vcpkg.cmake"
if (!(Test-Path $toolchainFile)) {
    throw "The vcpkg toolchain was not found at '$toolchainFile'. Check VCPKG_ROOT and run bootstrap-vcpkg if necessary."
}

Write-Host "Using VCPKG_ROOT: $env:VCPKG_ROOT"
Write-Host "Configuring preset: $ConfigurePreset"
cmake --preset $ConfigurePreset
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed." }

Write-Host "Building preset: $BuildPreset"
cmake --build --preset $BuildPreset
if ($LASTEXITCODE -ne 0) { throw "Build failed." }

if ($RunTests) {
    & "$PSScriptRoot\run-tests.ps1" -BuildDir ".build\$ConfigurePreset" -Config Debug
    if ($LASTEXITCODE -ne 0) { throw "Debug test run failed." }
} else {
    Write-Host "Done. To run tests: .\\ci\\dev\\run-tests.ps1 -BuildDir .build\\$ConfigurePreset -Config Debug"
}
