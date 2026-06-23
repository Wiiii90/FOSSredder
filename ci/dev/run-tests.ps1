# Developer helper for running CTest from a configured build directory.
param(
    [string]$BuildDir = ".build/tests",
    [ValidateSet('Debug','Release')][string]$Config = "Release",
    [string[]]$Labels
)

if ($Labels -and $Labels.Count -gt 0) {
    foreach ($label in $Labels) {
        Write-Host "Running $label tests"
        ctest --test-dir $BuildDir -C $Config -L $label --output-on-failure
        if ($LASTEXITCODE -ne 0) { throw "Tests failed for label '$label'." }
    }
} else {
    Write-Host "Running all registered tests"
    ctest --test-dir $BuildDir -C $Config --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "Tests failed." }
}

Write-Host "All tests passed."
