param(
    [string]$BuildDir = ".build\\app",
    [string]$Config = "Release",
    [string]$StagingDir = ".build\\app\\staging",
    [string]$InstallerScript = "installer\\inno\\fossredder.iss",
    [string]$ISCCPath = "C:\\Program Files (x86)\\Inno Setup 6\\ISCC.exe",
    [string]$OutputDir = ".build\\app\\dist",
    [string]$WizardImageFile = "installer\\assets\\wizard-image.bmp",
    [string]$WizardSmallFile = "installer\\assets\\wizard-small.bmp",
    [string]$LocalizationContract = "ci\\localization\\localization-contract.json",
    [string]$AppQmlSourceDir = "ui\\qml\\FossRedder",
    [string]$AppQmlToolingDir = "",
    [string]$Version = $env:PACKAGE_VERSION,
    [switch]$RunWindeployQt,
    [switch]$RunQtDeployFallback = $true,
    [switch]$CopyVcpkgRuntimeDeps = $true,
    [switch]$CleanStaging
)

if (-not $Version) {
    Write-Host "Version not provided via env PACKAGE_VERSION or -Version parameter. Using default 0.5.0" -ForegroundColor Yellow
    $Version = "0.5.0"
}

# Repo root is two levels above this script's directory (ci/package/../..)
$RepoRoot = (Resolve-Path -Path (Join-Path $PSScriptRoot "..\\..")).ProviderPath

function Get-AbsPath([string]$PathValue, [string]$BaseDir) {
    if ([string]::IsNullOrWhiteSpace($PathValue)) { return $null }
    if ([System.IO.Path]::IsPathRooted($PathValue)) {
        return [System.IO.Path]::GetFullPath($PathValue)
    }
    return [System.IO.Path]::GetFullPath((Join-Path -Path $BaseDir -ChildPath $PathValue))
}

function Get-CMakeCacheValue([string]$CachePath, [string]$Key) {
    if (!(Test-Path $CachePath)) { return $null }

    $prefix = "${Key}:"
    foreach ($line in Get-Content -Path $CachePath) {
        if ($line.StartsWith($prefix)) {
            $parts = $line.Split('=', 2)
            if ($parts.Length -eq 2) {
                return $parts[1]
            }
        }
    }

    return $null
}

function Assert-Path([string]$PathValue, [string]$Message) {
    if (!(Test-Path $PathValue)) {
        throw $Message
    }
}

function Convert-InstallerBitmap {
    param(
        [Parameter(Mandatory)][string]$SourcePath,
        [Parameter(Mandatory)][string]$OutputPath,
        [Parameter(Mandatory)][int]$Width,
        [Parameter(Mandatory)][int]$Height
    )

    Add-Type -AssemblyName System.Drawing

    $sourceImage = [System.Drawing.Image]::FromFile($SourcePath)
    try {
        $bitmap = New-Object System.Drawing.Bitmap $Width, $Height, ([System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
        try {
            $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
            try {
                $graphics.Clear([System.Drawing.Color]::White)
                $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
                $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
                $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality

                $scale = [Math]::Max($Width / $sourceImage.Width, $Height / $sourceImage.Height)
                $drawWidth = [int][Math]::Ceiling($sourceImage.Width * $scale)
                $drawHeight = [int][Math]::Ceiling($sourceImage.Height * $scale)
                $drawX = [int][Math]::Floor(($Width - $drawWidth) / 2)
                $drawY = [int][Math]::Floor(($Height - $drawHeight) / 2)
                $graphics.DrawImage($sourceImage, $drawX, $drawY, $drawWidth, $drawHeight)
            } finally {
                $graphics.Dispose()
            }

            $bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Bmp)
        } finally {
            $bitmap.Dispose()
        }
    } finally {
        $sourceImage.Dispose()
    }
}

$BuildDirAbs = Get-AbsPath $BuildDir $RepoRoot
$StagingDirAbs = Get-AbsPath $StagingDir $RepoRoot
$OutputDirAbs = Get-AbsPath $OutputDir $RepoRoot
$InstallerAbs = Get-AbsPath $InstallerScript $RepoRoot
$LicenseFileAbs = Get-AbsPath "LICENSE" $RepoRoot
$IconFileAbs = Get-AbsPath "app\\assets\\icons\\fossredder.ico" $RepoRoot
$WizardImageSourceAbs = Get-AbsPath $WizardImageFile $RepoRoot
$WizardSmallSourceAbs = Get-AbsPath $WizardSmallFile $RepoRoot
$LocalizationContractAbs = Get-AbsPath $LocalizationContract $RepoRoot
$AppQmlSourceDirAbs = Get-AbsPath $AppQmlSourceDir $RepoRoot

if (-not (Test-Path $BuildDirAbs)) { throw "BuildDir not found: $BuildDirAbs" }
if (-not (Test-Path $InstallerAbs)) { throw "Installer script not found: $InstallerAbs" }
if (-not (Test-Path $LicenseFileAbs)) { throw "License file not found: $LicenseFileAbs" }
if (-not (Test-Path $IconFileAbs)) { throw "Installer icon file not found: $IconFileAbs" }
if (-not (Test-Path $WizardImageSourceAbs)) { throw "Wizard image not found: $WizardImageSourceAbs" }
if (-not (Test-Path $WizardSmallSourceAbs)) { throw "Wizard small image not found: $WizardSmallSourceAbs" }
if (-not (Test-Path $LocalizationContractAbs)) { throw "Localization contract not found: $LocalizationContractAbs" }
if (-not (Test-Path $AppQmlSourceDirAbs)) { throw "App QML source directory not found: $AppQmlSourceDirAbs" }

if ([string]::IsNullOrWhiteSpace($AppQmlToolingDir)) {
    $AppQmlToolingDirAbs = Join-Path $BuildDirAbs "ui\qmltooling\FossRedder"
} else {
    $AppQmlToolingDirAbs = Get-AbsPath $AppQmlToolingDir $RepoRoot
}

# Clean staging to avoid leftovers (optional; CMake 'package' target might already manage staging)
if ($CleanStaging -and (Test-Path $StagingDirAbs)) {
    Write-Host "Removing existing staging directory: $StagingDirAbs" -ForegroundColor Cyan
    Remove-Item -Path $StagingDirAbs -Recurse -Force -ErrorAction SilentlyContinue
}
if (!(Test-Path $StagingDirAbs)) {
    New-Item -ItemType Directory -Path $StagingDirAbs | Out-Null
}

Write-Host "Creating/refreshing staging via CMake install"
cmake --install $BuildDirAbs --config $Config --prefix $StagingDirAbs
if ($LASTEXITCODE -ne 0) { throw "cmake --install failed" }

$ExePath = Join-Path $StagingDirAbs "bin\fossredder.exe"
$BinDir = Join-Path $StagingDirAbs "bin"

# Where we actually deploy Qt/plugins/qml and vcpkg runtime DLLs.
# For consistency with `cmake/modules/FossredderQtDeploy.cmake`, deploy into the exe directory (staging/bin)
# and keep qml/platforms paths relative to it.
$DeployDir = $BinDir

if (!(Test-Path $ExePath)) { throw "Executable not found at $ExePath. Build/install likely failed." }
if (!(Test-Path $BinDir)) { New-Item -ItemType Directory -Path $BinDir | Out-Null }

# vcpkg installed root
$cmakeCachePath = Join-Path $BuildDirAbs "CMakeCache.txt"
$vcpkgInstalled = Get-CMakeCacheValue $cmakeCachePath "VCPKG_INSTALLED_DIR"
if ([string]::IsNullOrWhiteSpace($vcpkgInstalled)) {
    $vcpkgInstalled = Join-Path $RepoRoot ".build\vcpkg_installed"
}
$vcpkgInstalled = Get-AbsPath $vcpkgInstalled $RepoRoot
if (!(Test-Path $vcpkgInstalled)) { throw "vcpkg installed dir not found: $vcpkgInstalled" }

# Automatically copy vcpkg runtime dependencies (transitive) into staging/bin.
# This avoids chasing missing DLLs one-by-one (opencv, poppler, tesseract, etc.).
# It uses `dumpbin /DEPENDENTS` to compute the closure from the staged exe and any DLLs already in staging/bin.
$vcpkgTriplet = Get-CMakeCacheValue $cmakeCachePath "VCPKG_TARGET_TRIPLET"
if ([string]::IsNullOrWhiteSpace($vcpkgTriplet)) {
    $vcpkgTriplet = "x64-windows"
}
$vcpkgBin = Join-Path $vcpkgInstalled "$vcpkgTriplet\bin"

function Get-DumpbinPath {
    $cmd = Get-Command dumpbin.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Path }

    if ($env:VCToolsInstallDir) {
        $cand = Join-Path $env:VCToolsInstallDir "bin\Hostx64\x64\dumpbin.exe"
        if (Test-Path $cand) { return $cand }
    }

    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
        if ($vsPath) {
            $dumpbin = Get-ChildItem -Path (Join-Path $vsPath "VC\Tools\MSVC") -Recurse -Filter dumpbin.exe -ErrorAction SilentlyContinue | Select-Object -First 1
            if ($dumpbin) { return $dumpbin.FullName }
        }
    }

    return $null
}

function Get-PeDependents([string]$Dumpbin, [string]$FilePath) {
    if (!(Test-Path $FilePath)) { return @() }
    $out = & $Dumpbin /nologo /dependents $FilePath 2>$null
    if ($LASTEXITCODE -ne 0 -or -not $out) { return @() }

    $deps = @()
    foreach ($line in $out) {
        $t = ($line | Out-String).Trim()
        if ($t -match '^[A-Za-z0-9_.-]+\.(dll|DLL)$') {
            $deps += $t
        }
    }
    return ($deps | Select-Object -Unique)
}

# Qt deployment
# - Preferred: deterministic cmake/modules/FossredderQtDeploy.cmake (same as ci/package/check-deploy.ps1)
# - Optional: windeployqt (useful when adding new Qt modules/plugins)

$logsDir = Join-Path $RepoRoot ".build\logs\package"
if (!(Test-Path $logsDir)) {
    New-Item -ItemType Directory -Path $logsDir | Out-Null
}

$installerAssetBuildDir = Join-Path $RepoRoot ".build\installer-assets"
if (!(Test-Path $installerAssetBuildDir)) {
    New-Item -ItemType Directory -Path $installerAssetBuildDir | Out-Null
}

$WizardImageBuildAbs = Join-Path $installerAssetBuildDir "wizard-image.bmp"
$WizardSmallBuildAbs = Join-Path $installerAssetBuildDir "wizard-small.bmp"
Convert-InstallerBitmap -SourcePath $WizardImageSourceAbs -OutputPath $WizardImageBuildAbs -Width 164 -Height 314
Convert-InstallerBitmap -SourcePath $WizardSmallSourceAbs -OutputPath $WizardSmallBuildAbs -Width 55 -Height 55

if ($RunQtDeployFallback) {
    $qtdeploy = Join-Path $RepoRoot "cmake\modules\FossredderQtDeploy.cmake"
    if (!(Test-Path $qtdeploy)) { throw "FossredderQtDeploy.cmake not found at $qtdeploy" }

    $vcpkgInstalledAbsForCmake = $vcpkgInstalled
    if (Test-Path $vcpkgInstalled) { $vcpkgInstalledAbsForCmake = (Resolve-Path $vcpkgInstalled).ProviderPath }

    Write-Host "Running deterministic FossredderQtDeploy.cmake" -ForegroundColor Cyan
    $qtDeployArgs = @(
        "-D", "TARGET_DIR=$DeployDir",
        "-D", "VCPKG_INSTALLED_DIR=$vcpkgInstalledAbsForCmake",
        "-D", "VCPKG_TARGET_TRIPLET=$vcpkgTriplet",
        "-D", "BUILD_CONFIG=$Config",
        "-D", "APP_QML_SOURCE_DIR=$AppQmlSourceDirAbs"
    )

    if (Test-Path $AppQmlToolingDirAbs) {
        $qtDeployArgs += @("-D", "APP_QML_TOOLING_DIR=$AppQmlToolingDirAbs")
    } else {
        Write-Host "[WARN] App QML tooling directory not found; continuing without qmltypes metadata: $AppQmlToolingDirAbs" -ForegroundColor Yellow
    }

    $qtDeployArgs += @("-P", $qtdeploy)
    & cmake @qtDeployArgs
    if ($LASTEXITCODE -ne 0) { throw "FossredderQtDeploy.cmake failed" }
}

# Automatically copy vcpkg runtime dependencies (transitive) into deploy dir.
# Run this AFTER Qt deploy so the scanner also sees Qt plugin DLLs already placed in staging/bin.
if ($CopyVcpkgRuntimeDeps) {
    if (!(Test-Path $vcpkgBin)) { throw "vcpkg bin dir not found: $vcpkgBin" }

    $dumpbin = Get-DumpbinPath
    if (-not $dumpbin) { throw "dumpbin.exe not found. Install Visual C++ tools or run from a VS Developer PowerShell." }

    Write-Host "[INFO] Using dumpbin: $dumpbin" -ForegroundColor Cyan

    # seed: staged exe + any dll already in staging/bin (Qt + plugins already copied)
    $seedFiles = @($ExePath)
    $seedFiles += (Get-ChildItem -Path $DeployDir -Filter *.dll -File -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName)
    $seedFiles = $seedFiles | Select-Object -Unique

    $seen = New-Object 'System.Collections.Generic.HashSet[string]'
    $queue = New-Object 'System.Collections.Generic.Queue[string]'

    foreach ($f in $seedFiles) {
        $full = [System.IO.Path]::GetFullPath($f)
        if ($seen.Add($full)) { $queue.Enqueue($full) }
    }

    $copied = 0
    while ($queue.Count -gt 0) {
        $current = $queue.Dequeue()
        foreach ($depName in (Get-PeDependents -Dumpbin $dumpbin -FilePath $current)) {
            $dest = Join-Path $DeployDir $depName
            if (Test-Path $dest) { continue }

            $src = Join-Path $vcpkgBin $depName
            if (!(Test-Path $src)) { continue }

            Copy-Item -Path $src -Destination $dest -Force
            $copied++

            $srcFull = [System.IO.Path]::GetFullPath($src)
            if ($seen.Add($srcFull)) { $queue.Enqueue($srcFull) }
        }
    }

    Write-Host "[OK] Copied $copied vcpkg runtime dependency DLL(s) into staging/bin" -ForegroundColor Green
}

# Use windeployqt from vcpkg only (deterministic)
$windeploy = $null

# Prefer explicit from cache/env if present
if ($env:WINDEPLOYQT_EXECUTABLE -and (Test-Path $env:WINDEPLOYQT_EXECUTABLE)) {
    $windeploy = $env:WINDEPLOYQT_EXECUTABLE
}

if (-not $windeploy -and (Test-Path $vcpkgInstalled)) {
    $found = Get-ChildItem -Path $vcpkgInstalled -Recurse -Filter windeployqt.exe -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) { $windeploy = $found.FullName }
}

if ($RunWindeployQt) {
    if (-not $windeploy) {
        throw "windeployqt.exe not found under vcpkg installed dir ($vcpkgInstalled) and WINDEPLOYQT_EXECUTABLE not set."
    }

    # QML dir: prefer installed bin/qml (comes from cmake install), then ui/qml source
    $qmlDir = Join-Path $DeployDir "qml"
    if (!(Test-Path $qmlDir)) {
        $uiQml = Join-Path $RepoRoot "ui\qml"
        if (Test-Path $uiQml) { $qmlDir = $uiQml }
    }

    Write-Host "Using windeployqt: $windeploy" -ForegroundColor Cyan
    Write-Host "Using qmldir: $qmlDir" -ForegroundColor Cyan

    $windeployLog = Join-Path $logsDir "windeployqt-output.txt"
    $windeployArgs = @()
    if ($Config -eq 'Release') { $windeployArgs += '--release' }

    & $windeploy $ExePath $windeployArgs --qmldir="$qmlDir" 2>&1 | Tee-Object -FilePath $windeployLog
    $windeployExit = $LASTEXITCODE
    if ($windeployExit -ne 0) { throw "windeployqt failed with exit code $windeployExit. See $windeployLog" }
}

# Sanity check: Qt dlls present if either deploy method ran
$qtDlls = Get-ChildItem -Path $DeployDir -Filter "Qt6*.dll" -File -ErrorAction SilentlyContinue
if ($RunQtDeployFallback -or $RunWindeployQt) {
    if (($qtDlls -eq $null) -or ($qtDlls.Count -eq 0)) {
        throw "Qt deployment was requested but no Qt6 DLLs were found in $BinDir"
    }

    Assert-Path (Join-Path $DeployDir 'qt.conf') "qt.conf missing in staging/bin after Qt deploy"
    Assert-Path (Join-Path $DeployDir 'platforms\\qwindows.dll') "qwindows.dll missing in staging/bin/platforms after Qt deploy"
    Assert-Path (Join-Path $DeployDir 'qml\\QtQuick\\qmldir') "QML QtQuick qmldir missing in staging/bin/qml after Qt deploy"
    Assert-Path (Join-Path $DeployDir 'qml\\FossRedder\\qmldir') "App QML root qmldir missing in staging/bin/qml after Qt deploy"
}

$tessdataDir = Join-Path $DeployDir 'res\tessdata'
Assert-Path $tessdataDir "Tesseract tessdata directory missing in staging/bin/res/tessdata"

$localizationContractData = Get-Content -Path $LocalizationContractAbs -Raw | ConvertFrom-Json
$expectedTessdataModels = @()
if ($localizationContractData.ocrModels.required) {
    $expectedTessdataModels += @($localizationContractData.ocrModels.required)
}
if ($localizationContractData.ocrModels.optional) {
    $expectedTessdataModels += @($localizationContractData.ocrModels.optional)
}
$expectedTessdataModels = @($expectedTessdataModels | Select-Object -Unique)

foreach ($model in $expectedTessdataModels) {
    Assert-Path (Join-Path $tessdataDir ("{0}.traineddata" -f $model)) "Tesseract model '$model' missing in staging/bin/res/tessdata"
}

if (!(Test-Path $ISCCPath)) { throw "ISCC not found at $ISCCPath. Ensure Inno Setup is installed." }
if (!(Test-Path $OutputDirAbs)) { New-Item -ItemType Directory -Path $OutputDirAbs | Out-Null }

# If a previous installer exe is open (e.g. in Explorer), ISCC may fail with Error 1224.
# Remove the target output file up-front so compilation can proceed reliably.
$expectedInstaller = Join-Path $OutputDirAbs ("FOSSredder-Setup-{0}-win-x64.exe" -f $Version)
if (Test-Path $expectedInstaller) {
    try {
        Remove-Item -Path $expectedInstaller -Force -ErrorAction Stop
    } catch {
        Write-Host "[WARN] Could not remove existing installer '$expectedInstaller'. Close any open instance and retry." -ForegroundColor Yellow
    }
}

$isccLog = Join-Path $logsDir "iscc-output.txt"
$isccOut = Join-Path $logsDir "iscc-out.txt"
$isccErr = Join-Path $logsDir "iscc-err.txt"
$arguments = @(
    $InstallerAbs,
    "/DOutputDir=`"$OutputDirAbs`"",
    "/DVersion=`"$Version`"",
    "/DStaging=`"$StagingDirAbs`"",
    "/DLicenseFile=`"$LicenseFileAbs`"",
    "/DIconFile=`"$IconFileAbs`"",
    "/DWizardImageFile=`"$WizardImageBuildAbs`"",
    "/DWizardSmallFile=`"$WizardSmallBuildAbs`""
)
$proc = Start-Process -FilePath $ISCCPath -ArgumentList $arguments -NoNewWindow -Wait -PassThru -RedirectStandardOutput $isccOut -RedirectStandardError $isccErr
Get-Content $isccOut, $isccErr | Out-File -FilePath $isccLog -Encoding utf8
if ($proc.ExitCode -ne 0) { throw "ISCC failed with exit code $($proc.ExitCode). See $isccLog" }

Write-Host "Installer(s) should be in: $OutputDirAbs"
