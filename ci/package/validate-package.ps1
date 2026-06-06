param(
    [string]$DistDir = ".build\\app\\dist",
    [string]$Version = $env:PACKAGE_VERSION,
    [int64]$MinimumSizeBytes = 1MB
)

$RepoRoot = (Resolve-Path -Path (Join-Path $PSScriptRoot "..\\..")).ProviderPath

function Get-AbsPath([string]$PathValue, [string]$BaseDir) {
    if ([string]::IsNullOrWhiteSpace($PathValue)) { return $null }
    if ([System.IO.Path]::IsPathRooted($PathValue)) {
        return [System.IO.Path]::GetFullPath($PathValue)
    }
    return [System.IO.Path]::GetFullPath((Join-Path -Path $BaseDir -ChildPath $PathValue))
}

if (-not $Version) {
    $Version = "0.1.0"
}

$DistDirAbs = Get-AbsPath $DistDir $RepoRoot
if (!(Test-Path $DistDirAbs)) {
    throw "Package output directory not found: $DistDirAbs"
}

$expectedName = "FOSSredder-Setup-$Version-win-x64.exe"
$expectedPath = Join-Path $DistDirAbs $expectedName
if (!(Test-Path $expectedPath)) {
    $available = Get-ChildItem -Path $DistDirAbs -File -ErrorAction SilentlyContinue |
        Select-Object -ExpandProperty Name
    throw "Expected installer not found: $expectedPath. Available files: $($available -join ', ')"
}

$installer = Get-Item -Path $expectedPath
if ($installer.Length -lt $MinimumSizeBytes) {
    throw "Installer is unexpectedly small: $($installer.Length) bytes. Expected at least $MinimumSizeBytes bytes."
}

Write-Host "Package validation ok."
Write-Host "Installer: $($installer.FullName)"
Write-Host "Size: $($installer.Length) bytes"

if ($env:GITHUB_STEP_SUMMARY) {
    @(
        "## Package artifact",
        "",
        "| Field | Value |",
        "| --- | --- |",
        "| Installer | `$($installer.Name)` |",
        "| Size | `$($installer.Length)` bytes |"
    ) | Add-Content -Path $env:GITHUB_STEP_SUMMARY
}
