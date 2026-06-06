# Upload a generated LCOV report to Codecov.
param(
    [string]$CoverageFile = "coverage/coverage.lcov",
    [string]$RepositorySlug = $env:GITHUB_REPOSITORY,
    [string]$CommitSha = $env:GITHUB_SHA,
    [string]$Branch = $(if ($env:GITHUB_HEAD_REF) { $env:GITHUB_HEAD_REF } elseif ($env:GITHUB_REF_NAME) { $env:GITHUB_REF_NAME } else { "master" }),
    [string]$UploadName = $(if ($env:GITHUB_RUN_ID) { "coverage-$($env:GITHUB_RUN_ID)" } else { "coverage-local" }),
    [string]$UploaderVersion = "latest",
    [switch]$DryRun
)

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$coveragePath = if ([System.IO.Path]::IsPathRooted($CoverageFile)) { $CoverageFile } else { Join-Path $repoRoot $CoverageFile }

if (!(Test-Path $coveragePath)) {
    throw "Coverage file not found: $coveragePath"
}

if ([string]::IsNullOrWhiteSpace($RepositorySlug)) {
    $RepositorySlug = ((git config --get remote.origin.url) -replace '^https://github.com/', '' -replace '\.git$', '')
}
if ([string]::IsNullOrWhiteSpace($CommitSha)) {
    $CommitSha = (git rev-parse HEAD).Trim()
}
if ([string]::IsNullOrWhiteSpace($Branch)) {
    $Branch = (git rev-parse --abbrev-ref HEAD).Trim()
}

$runnerTemp = if ($env:RUNNER_TEMP) { $env:RUNNER_TEMP } else { Join-Path $repoRoot ".tmp" }
$uploaderDir = Join-Path $runnerTemp "codecov-uploader"
$uploaderExe = Join-Path $uploaderDir "codecov.exe"
$checksumFile = Join-Path $uploaderDir "codecov.exe.SHA256SUM"
$downloadRoot = "https://uploader.codecov.io/$UploaderVersion/windows"

New-Item -ItemType Directory -Path $uploaderDir -Force | Out-Null

Write-Host "Downloading Codecov uploader"
Invoke-WebRequest -Uri "$downloadRoot/codecov.exe" -OutFile $uploaderExe
Invoke-WebRequest -Uri "$downloadRoot/codecov.exe.SHA256SUM" -OutFile $checksumFile

$expectedHash = ((Get-Content $checksumFile | Select-Object -First 1).Split(' ', [System.StringSplitOptions]::RemoveEmptyEntries)[0]).Trim().ToLowerInvariant()
$actualHash = (Get-FileHash -Path $uploaderExe -Algorithm SHA256).Hash.ToLowerInvariant()
if ($expectedHash -ne $actualHash) {
    throw "Codecov uploader checksum mismatch. Expected $expectedHash but got $actualHash"
}

$arguments = @(
    '-f', (Resolve-Path $coveragePath).Path,
    '-n', $UploadName,
    '-Q', 'github-action/self-hosted-windows',
    '-Z',
    '-v'
)

$effectiveRepositorySlug = if ([string]::IsNullOrWhiteSpace($env:GITHUB_REPOSITORY)) {
    $RepositorySlug
} else {
    $env:GITHUB_REPOSITORY
}
$arguments += @('-r', $effectiveRepositorySlug.ToLowerInvariant())

$arguments += @('-C', $CommitSha)
$arguments += @('-B', $Branch)

$envNames = @('GITHUB_RUN_ID', 'GITHUB_RUN_ATTEMPT', 'GITHUB_WORKFLOW') |
    Where-Object { ![string]::IsNullOrWhiteSpace([Environment]::GetEnvironmentVariable($_)) }
if ($envNames.Count -gt 0) {
    $arguments += @('-e', ($envNames -join ','))
}

if ($env:GITHUB_REF_TYPE -eq 'tag' -and ![string]::IsNullOrWhiteSpace($env:GITHUB_REF_NAME)) {
    $arguments += @('-T', $env:GITHUB_REF_NAME)
}

if (!([string]::IsNullOrWhiteSpace($env:CODECOV_TOKEN))) {
    $arguments += @('-t', $env:CODECOV_TOKEN)
}

if ($DryRun) {
    $arguments += '-d'
}

Write-Host "Uploading coverage for $RepositorySlug @ $CommitSha on branch $Branch"
& $uploaderExe @arguments
if ($LASTEXITCODE -ne 0) {
    throw "Codecov uploader failed with exit code $LASTEXITCODE"
}
