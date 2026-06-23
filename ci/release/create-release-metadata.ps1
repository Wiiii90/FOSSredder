param(
    [string]$DistDir = ".build\app\dist",
    [string]$Version = $env:PACKAGE_VERSION,
    [string]$OutDir = ".build\app\release"
)

$RepoRoot = (Resolve-Path -Path (Join-Path $PSScriptRoot "..\..")).ProviderPath

function Get-AbsPath([string]$PathValue) {
    if ([System.IO.Path]::IsPathRooted($PathValue)) {
        return [System.IO.Path]::GetFullPath($PathValue)
    }
    return [System.IO.Path]::GetFullPath((Join-Path -Path $RepoRoot -ChildPath $PathValue))
}

if (-not $Version) {
    $Version = "0.5.0"
}

$releaseRef = $env:RELEASE_REF_NAME
if ([string]::IsNullOrWhiteSpace($releaseRef)) {
    $releaseRef = $env:GITHUB_REF_NAME
}

$releaseCommit = $env:RELEASE_COMMIT_SHA
if ([string]::IsNullOrWhiteSpace($releaseCommit)) {
    $releaseCommit = $env:GITHUB_SHA
}

$distPath = Get-AbsPath $DistDir
$outPath = Get-AbsPath $OutDir
if (!(Test-Path $distPath)) {
    throw "Release artifact directory not found: $distPath"
}
if (!(Test-Path $outPath)) {
    New-Item -ItemType Directory -Path $outPath | Out-Null
}

$artifacts = @(Get-ChildItem -Path $distPath -File -ErrorAction SilentlyContinue)
if ($artifacts.Count -eq 0) {
    throw "No release artifacts found in: $distPath"
}

$manifestArtifacts = @()
$checksumLines = @()
foreach ($artifact in $artifacts) {
    $hash = Get-FileHash -Path $artifact.FullName -Algorithm SHA256
    $checksumLines += "$($hash.Hash.ToLowerInvariant())  $($artifact.Name)"
    $manifestArtifacts += [ordered]@{
        name = $artifact.Name
        size = $artifact.Length
        sha256 = $hash.Hash.ToLowerInvariant()
    }
}

$manifest = [ordered]@{
    product = "FOSSredder"
    version = $Version
    branch = $releaseRef
    commit = $releaseCommit
    run = $env:GITHUB_RUN_ID
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    artifacts = $manifestArtifacts
}

$checksumsPath = Join-Path $outPath "SHA256SUMS.txt"
$manifestPath = Join-Path $outPath "release-manifest.json"
$notesPath = Join-Path $outPath "release-notes.md"

$checksumLines | Set-Content -Path $checksumsPath -Encoding utf8
$manifest | ConvertTo-Json -Depth 8 | Set-Content -Path $manifestPath -Encoding utf8

@(
    "FOSSredder $Version",
    "",
    "Windows x64 installer release candidate.",
    "",
    "- Commit: $releaseCommit",
    "- Workflow run: $env:GITHUB_SERVER_URL/$env:GITHUB_REPOSITORY/actions/runs/$env:GITHUB_RUN_ID",
    "- Checksums: see ``SHA256SUMS.txt``"
) | Set-Content -Path $notesPath -Encoding utf8

Write-Host "Release metadata created:"
Write-Host " - $checksumsPath"
Write-Host " - $manifestPath"
Write-Host " - $notesPath"

if ($env:GITHUB_STEP_SUMMARY) {
    @(
        "## Release metadata",
        "",
        "| File | Purpose |",
        "| --- | --- |",
        "| ``SHA256SUMS.txt`` | Installer integrity checksums |",
        "| ``release-manifest.json`` | Machine-readable release metadata |",
        "| ``release-notes.md`` | Draft release notes |"
    ) | Add-Content -Path $env:GITHUB_STEP_SUMMARY
}
