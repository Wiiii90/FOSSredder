param(
    [string]$ContractFile = "ci\localization\localization-contract.json",
    [string]$TranslationsDir = "app\i18n",
    [string]$TessdataDir = "infra\text-recognition\res\tessdata"
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

$contractPath = Get-AbsPath $ContractFile
$translationsPath = Get-AbsPath $TranslationsDir
$tessdataPath = Get-AbsPath $TessdataDir

Assert-Path $contractPath "Localization contract not found: $contractPath"
Assert-Path $translationsPath "Translations directory not found: $translationsPath"
Assert-Path $tessdataPath "Tesseract tessdata directory not found: $tessdataPath"

$contract = Get-Content -Path $contractPath -Raw | ConvertFrom-Json
$summaryRows = New-Object System.Collections.Generic.List[string]
$summaryRows.Add("| Check | Result |")
$summaryRows.Add("| --- | --- |")

foreach ($language in $contract.uiLanguages) {
    if ($language.code -eq "en") {
        $summaryRows.Add(("| UI ``{0}`` | built-in source language |" -f $language.code))
        continue
    }

    $tsFile = Join-Path $translationsPath ("fossredder_{0}.ts" -f $language.code)
    Assert-Path $tsFile "Translation source missing for UI language '$($language.code)': $tsFile"

    [xml]$document = Get-Content -Path $tsFile -Raw
    $actualLanguage = $document.TS.language
    if ($actualLanguage -ne $language.qtLanguage) {
        throw "Translation '$tsFile' declares language '$actualLanguage', expected '$($language.qtLanguage)'."
    }

    $unfinished = @($document.SelectNodes("//translation[@type='unfinished']"))
    if ($unfinished.Count -gt 0) {
        throw "Translation '$tsFile' contains $($unfinished.Count) unfinished message(s)."
    }

    $messages = @($document.SelectNodes("//message"))
    $summaryRows.Add(("| UI ``{0}`` | {1} messages, no unfinished translations |" -f $language.code, $messages.Count))
}

foreach ($model in $contract.ocrModels.required) {
    $modelPath = Join-Path $tessdataPath ("{0}.traineddata" -f $model)
    Assert-Path $modelPath "Required Tesseract OCR model missing: $modelPath"
    $summaryRows.Add(("| OCR ``{0}`` | bundled |" -f $model))
}

$optionalModels = @()
if ($contract.ocrModels.optional) {
    $optionalModels = @($contract.ocrModels.optional)
}

foreach ($model in $optionalModels) {
    $modelPath = Join-Path $tessdataPath ("{0}.traineddata" -f $model)
    if (Test-Path $modelPath) {
        $summaryRows.Add(("| OCR ``{0}`` | bundled optional model |" -f $model))
    } else {
        $summaryRows.Add(("| OCR ``{0}`` | optional model not bundled |" -f $model))
    }
}

Write-Host "Localization contract validation ok."
$summaryRows | ForEach-Object { Write-Host $_ }

if ($env:GITHUB_STEP_SUMMARY) {
    @("## Localization", "") + $summaryRows | Add-Content -Path $env:GITHUB_STEP_SUMMARY
}
