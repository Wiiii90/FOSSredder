# Developer helper for checking staged Qt deployment output.
param(
    [string]$BuildDir = ".\\.build\\app",
    [string]$StagingDir = ".\\.build\\app\\staging",
    [string]$VcpkgInstalled = "",
    [string]$Config = "Release"
)

if ([string]::IsNullOrWhiteSpace($VcpkgInstalled)) {
    if ($env:VCPKG_INSTALLED_DIR) {
        $VcpkgInstalled = $env:VCPKG_INSTALLED_DIR
    }
    elseif ($env:VCPKG_ROOT) {
        $VcpkgInstalled = Join-Path $env:VCPKG_ROOT "installed\fossredder"
    }
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).ProviderPath
$buildAbs = if (Test-Path $BuildDir) { (Resolve-Path $BuildDir).ProviderPath } else { [System.IO.Path]::GetFullPath((Join-Path $repoRoot $BuildDir)) }
$logsDir = Join-Path $repoRoot ".build\logs\package-check"
if (!(Test-Path $logsDir)) { New-Item -ItemType Directory -Path $logsDir | Out-Null }

Write-Host "[CHECK] BuildDir: $buildAbs"
Write-Host "[CHECK] StagingDir: $StagingDir"
Write-Host "[CHECK] VcpkgInstalled: $VcpkgInstalled"

# 1) Verify staging exe
$stagingAbs = $null
if (Test-Path $StagingDir) { $stagingAbs = (Resolve-Path $StagingDir).ProviderPath } else { $stagingAbs = $StagingDir }
$exe = Join-Path $stagingAbs "bin\fossredder.exe"
$deployAbs = Join-Path $stagingAbs "bin"
if (Test-Path $exe) { Write-Host "[OK] Found exe: $exe" } else { Write-Host "[ERROR] fossredder.exe not found in staging/bin" -ForegroundColor Red }

# 2) Find windeployqt
$candidates = @()
$pathCmd = Get-Command windeployqt.exe -ErrorAction SilentlyContinue
if ($pathCmd) { $candidates += $pathCmd.Path }
if ($env:QTDIR) {
    $cand = Join-Path $env:QTDIR "bin\windeployqt.exe"
    if (Test-Path $cand) { $candidates += $cand }
}
if (Test-Path $VcpkgInstalled) {
    $found = Get-ChildItem -Path $VcpkgInstalled -Recurse -Filter windeployqt.exe -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) { $candidates += $found.FullName }
}
$candidates = $candidates | Select-Object -Unique

if ($candidates.Count -eq 0) { Write-Host "[WARN] No windeployqt candidates found" -ForegroundColor Yellow } else { Write-Host "[OK] windeployqt candidates:"; $candidates | ForEach-Object { Write-Host " - $_" } }

# 3) Run windeployqt against staging exe (if found)
if ($candidates.Count -gt 0 -and (Test-Path $exe)) {
    $used = $candidates[0]
    Write-Host "[ACTION] Running windeployqt: $used on $exe"
    $outFile = Join-Path $logsDir "windeploy-log.txt"
    & $used $exe --qmldir="$deployAbs\qml" *> $outFile 2>&1
    $rc = $LASTEXITCODE
    Write-Host "[RESULT] windeployqt exit code: $rc. Log: $outFile"
} else { Write-Host "[SKIP] Skipping windeployqt run (no candidate or exe missing)" }

# 4) Run cmake Qt deploy fallback
Write-Host "[ACTION] Running cmake/modules/FossredderQtDeploy.cmake fallback"
$qtdeployLog = Join-Path $logsDir "qtdeploy-log.txt"

$vcpkgInstalledAbs = $VcpkgInstalled
if (Test-Path $VcpkgInstalled) {
    $vcpkgInstalledAbs = (Resolve-Path $VcpkgInstalled).ProviderPath
}

$qtDeployArgs = @(
    "-D", "TARGET_DIR=$deployAbs",
    "-D", "VCPKG_INSTALLED_DIR=$vcpkgInstalledAbs",
    "-D", "VCPKG_TARGET_TRIPLET=x64-windows",
    "-D", "BUILD_CONFIG=$Config",
    "-D", "APP_QML_SOURCE_DIR=$(Join-Path $repoRoot 'ui\qml\FossRedder')",
    "-D", "APP_QML_TOOLING_DIR=$(Join-Path $buildAbs 'ui\qmltooling\FossRedder')",
    "-P", "$repoRoot\cmake\modules\FossredderQtDeploy.cmake"
)
& cmake @qtDeployArgs *> $qtdeployLog 2>&1
$rc2 = $LASTEXITCODE
Write-Host "[RESULT] Qt deploy fallback exit code: $rc2. Log: $qtdeployLog"

# 5) List staging qml dir
$qmlDir = Join-Path $deployAbs "qml"
if (Test-Path $qmlDir) {
    Write-Host "[OK] staging qml files (first 200):"
    Get-ChildItem -Path $qmlDir -Recurse -Force | Select-Object -First 200 | ForEach-Object { Write-Host " - $($_.FullName)" }
} else {
    Write-Host "[WARN] staging qml dir not found: $qmlDir" -ForegroundColor Yellow
}

Write-Host "[INFO] Logs written to: $logsDir"
