param(
    [string]$StagingDir = ".build\app\staging",
    [string]$DistDir = ".build\app\dist",
    [string]$Version = $env:PACKAGE_VERSION,
    [string]$LocalizationContract = "ci\localization\localization-contract.json",
    [string]$SourceQmlDir = "ui\qml",
    [string]$SourceTessdataDir = "infra\text-recognition\res\tessdata"
)

$RepoRoot = (Resolve-Path -Path (Join-Path $PSScriptRoot "..\..")).ProviderPath

function Get-AbsPath([string]$PathValue) {
    if ([System.IO.Path]::IsPathRooted($PathValue)) {
        return [System.IO.Path]::GetFullPath($PathValue)
    }
    return [System.IO.Path]::GetFullPath((Join-Path -Path $RepoRoot -ChildPath $PathValue))
}

function Assert-Path([string]$PathValue, [string]$Message) {
    if (!(Test-Path $PathValue)) {
        throw $Message
    }
}

if (-not $Version) {
    $Version = "0.5.0"
}

$stagingPath = Get-AbsPath $StagingDir
$distPath = Get-AbsPath $DistDir
$contractPath = Get-AbsPath $LocalizationContract
$sourceQmlPath = Get-AbsPath $SourceQmlDir
$sourceTessdataPath = Get-AbsPath $SourceTessdataDir

Assert-Path $stagingPath "Package staging directory not found: $stagingPath"
Assert-Path $distPath "Package output directory not found: $distPath"
Assert-Path $contractPath "Localization contract not found: $contractPath"
Assert-Path $sourceQmlPath "Source QML directory not found: $sourceQmlPath"
Assert-Path $sourceTessdataPath "Source tessdata directory not found: $sourceTessdataPath"

$contract = Get-Content -Path $contractPath -Raw | ConvertFrom-Json
$binPath = Join-Path $stagingPath "bin"
$exePath = Join-Path $binPath "fossredder.exe"
$installerPath = Join-Path $distPath ("FOSSredder-Setup-{0}-win-x64.exe" -f $Version)

Assert-Path $exePath "Staged executable missing: $exePath"
Assert-Path $installerPath "Installer artifact missing: $installerPath"
Assert-Path (Join-Path $binPath "qt.conf") "qt.conf missing from staged bin directory."
Assert-Path (Join-Path $binPath "platforms\qwindows.dll") "Qt Windows platform plugin missing from staged package."
Assert-Path (Join-Path $binPath "qml\QtQuick\qmldir") "QtQuick QML import missing from staged package."
Assert-Path (Join-Path $binPath "qml\QtQuick\Controls\qmldir") "QtQuick Controls QML import missing from staged package."
Assert-Path (Join-Path $binPath "qml\QtQuick\Layouts\qmldir") "QtQuick Layouts QML import missing from staged package."
Assert-Path (Join-Path $binPath "qml\QtQuick\Effects\qmldir") "QtQuick Effects QML import missing from staged package."

$sourceQmldirs = @(Get-ChildItem -Path $sourceQmlPath -Recurse -Filter "qmldir" -File)
if ($sourceQmldirs.Count -eq 0) {
    throw "No source QML module descriptors found under: $sourceQmlPath"
}

foreach ($sourceQmldir in $sourceQmldirs) {
    $relativeModuleDir = [System.IO.Path]::GetRelativePath($sourceQmlPath, $sourceQmldir.DirectoryName)
    $stagedQmldirPath = Join-Path $binPath (Join-Path "qml" (Join-Path $relativeModuleDir "qmldir"))
    Assert-Path $stagedQmldirPath "FOSSredder QML module descriptor missing from staged package: $stagedQmldirPath"
}

$deployedQmlFiles = @(Get-ChildItem -Path (Join-Path $binPath "qml") -Recurse -Filter "*.qml" -File -ErrorAction SilentlyContinue)
if ($deployedQmlFiles.Count -eq 0) {
    throw "FOSSredder QML files missing from staged package."
}

$qtDlls = @(Get-ChildItem -Path $binPath -Filter "Qt6*.dll" -File -ErrorAction SilentlyContinue)
if ($qtDlls.Count -eq 0) {
    throw "No Qt6 runtime DLLs found in staged bin directory."
}

foreach ($language in $contract.uiLanguages) {
    if ($language.code -eq "en") {
        continue
    }
    $qmPath = Join-Path $binPath ("i18n\fossredder_{0}.qm" -f $language.code)
    Assert-Path $qmPath "Compiled Qt translation catalog missing: $qmPath"
}

foreach ($model in $contract.ocrModels.required) {
    $modelPath = Join-Path $binPath ("res\tessdata\{0}.traineddata" -f $model)
    Assert-Path $modelPath "Required Tesseract OCR model missing from staged package: $modelPath"
}

$optionalModels = @()
if ($contract.ocrModels.optional) {
    $optionalModels = @($contract.ocrModels.optional)
}

foreach ($model in $optionalModels) {
    $sourceModelPath = Join-Path $sourceTessdataPath ("{0}.traineddata" -f $model)
    $stagedModelPath = Join-Path $binPath ("res\tessdata\{0}.traineddata" -f $model)
    if (Test-Path $sourceModelPath) {
        Assert-Path $stagedModelPath "Optional Tesseract OCR model exists in source but is missing from staged package: $stagedModelPath"
    }
}

$sourceTessdataFiles = @(Get-ChildItem -Path $sourceTessdataPath -Filter "*.traineddata" -File)
foreach ($sourceModel in $sourceTessdataFiles) {
    $stagedModelPath = Join-Path $binPath ("res\tessdata\{0}" -f $sourceModel.Name)
    Assert-Path $stagedModelPath "Bundled Tesseract OCR model missing from staged package: $stagedModelPath"
}

Assert-Path (Join-Path $RepoRoot "installer\assets\wizard-banner.bmp") "Installer wizard banner asset missing."
Assert-Path (Join-Path $RepoRoot "installer\assets\wizard-small.bmp") "Installer wizard small image asset missing."

$installer = Get-Item -Path $installerPath
$rows = @(
    "| Check | Result |",
    "| --- | --- |",
    "| Executable | `bin/fossredder.exe` |",
    "| Qt runtime | $($qtDlls.Count) Qt DLL(s), platform plugin and QML imports |",
    "| App QML | $($deployedQmlFiles.Count) QML file(s), $($sourceQmldirs.Count) module descriptor(s) staged |",
    "| UI translations | compiled `.qm` catalogs for non-English UI languages |",
    "| OCR models | $($sourceTessdataFiles.Count) Tesseract model file(s) bundled |",
    "| Installer | `$($installer.Name)`, $($installer.Length) bytes |"
)

Write-Host "Installer QA layout validation ok."
$rows | ForEach-Object { Write-Host $_ }

if ($env:GITHUB_STEP_SUMMARY) {
    @("## Installer QA", "") + $rows | Add-Content -Path $env:GITHUB_STEP_SUMMARY
}
