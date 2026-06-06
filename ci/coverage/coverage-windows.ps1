# Collect LLVM coverage for the Windows CI coverage job.
param(
    [string]$BuildDir = ".build/coverage",
    [ValidateSet('Debug','Release')][string]$Config = "Release",
    [string]$OutDir = "coverage",
    [string]$LlvmCovExe,
    [string]$LlvmProfdataExe
)

function Resolve-ExecutablePath {
    param(
        [Parameter(Mandatory)][string]$CommandName,
        [string]$ExplicitPath,
        [string]$EnvironmentVariable
    )

    if ($ExplicitPath) {
        if (Test-Path $ExplicitPath) {
            return (Resolve-Path $ExplicitPath).Path
        }

        throw "$CommandName not found at: $ExplicitPath"
    }

    if ($EnvironmentVariable) {
        $envPath = [Environment]::GetEnvironmentVariable($EnvironmentVariable)
        if ($envPath) {
            if (Test-Path $envPath) {
                return (Resolve-Path $envPath).Path
            }

            throw "$CommandName not found at path from environment variable ${EnvironmentVariable}: $envPath"
        }
    }

    $command = Get-Command $CommandName -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($command) {
        return $command.Path
    }

    $clang = Get-Command clang-cl.exe -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($clang) {
        $candidate = Join-Path (Split-Path -Parent $clang.Path) $CommandName
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    throw "$CommandName could not be resolved from PATH, clang-cl, or environment overrides."
}

function Remove-CoverageNoise {
    param(
        [string[]]$Lines
    )

    return @($Lines | Where-Object { $_ -notmatch '^warning: \d+ functions have mismatched data$' })
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$resolvedBuildDir = (Resolve-Path $BuildDir).Path
$resolvedOutDir = Join-Path $repoRoot $OutDir
$profilesDir = Join-Path $resolvedOutDir "profiles"
$htmlDir = Join-Path $resolvedOutDir "html"
$summaryDir = Join-Path $resolvedOutDir "summary"
$lcovDir = Join-Path $resolvedOutDir "lcov"
$profdataFile = Join-Path $resolvedOutDir "fossredder-coverage.profdata"
$summaryFile = Join-Path $resolvedOutDir "coverage-summary.txt"
$lcovFile = Join-Path $resolvedOutDir "coverage.lcov"

$llvmCovPath = Resolve-ExecutablePath -CommandName "llvm-cov.exe" -ExplicitPath $LlvmCovExe -EnvironmentVariable "LLVM_COV"
$llvmProfdataPath = Resolve-ExecutablePath -CommandName "llvm-profdata.exe" -ExplicitPath $LlvmProfdataExe -EnvironmentVariable "LLVM_PROFDATA"

if (Test-Path $resolvedOutDir) {
    Remove-Item -Path $resolvedOutDir -Recurse -Force
}

New-Item -ItemType Directory -Path $profilesDir -Force | Out-Null
New-Item -ItemType Directory -Path $htmlDir -Force | Out-Null
New-Item -ItemType Directory -Path $summaryDir -Force | Out-Null
New-Item -ItemType Directory -Path $lcovDir -Force | Out-Null

$profilePattern = Join-Path $profilesDir "fossredder-%p-%m.profraw"
$testBinDir = Join-Path $resolvedBuildDir "bin\$Config"

if (!(Test-Path $testBinDir)) {
    throw "Test binary directory not found: $testBinDir"
}

$testBinaries = Get-ChildItem -Path $testBinDir -Filter "*tests*.exe" -File | Sort-Object FullName
if (!$testBinaries) {
    throw "No instrumented test executables found under: $testBinDir"
}

$previousProfileFile = $env:LLVM_PROFILE_FILE
$env:LLVM_PROFILE_FILE = $profilePattern

try {
    Write-Host "Running all registered tests with LLVM coverage instrumentation"
    ctest --test-dir $resolvedBuildDir -C $Config --output-on-failure
    if ($LASTEXITCODE -ne 0) {
        throw "CTest failed during coverage execution."
    }
}
finally {
    $env:LLVM_PROFILE_FILE = $previousProfileFile
}

$rawProfiles = Get-ChildItem -Path $profilesDir -Filter "*.profraw" -File | Sort-Object FullName
if (!$rawProfiles) {
    throw "No .profraw coverage files were generated under: $profilesDir"
}

Write-Host "Merging coverage profiles"
& $llvmProfdataPath merge -sparse ($rawProfiles.FullName) -o $profdataFile
if ($LASTEXITCODE -ne 0) {
    throw "llvm-profdata merge failed."
}

$ignoreRegex = [string]::Join('|', @(
    '[\\/]tests[\\/]',
    '[\\/]autogen[\\/]',
    '[\\/]CMakeFiles[\\/]',
    '[\\/]moc_.*\\.cpp$',
    '[\\/]qrc_.*\\.cpp$',
    '[\\/]vcpkg_installed[\\/]',
    '[\\/]vcpkg[\\/]installed[\\/]',
    '[\\/]_deps[\\/]'
))

$commonCoverageArgs = @(
    "-instr-profile",
    $profdataFile,
    "-ignore-filename-regex",
    $ignoreRegex,
    "-path-equivalence",
    "$repoRoot,."
)

$summarySections = New-Object System.Collections.Generic.List[string]
$htmlIndexEntries = New-Object System.Collections.Generic.List[string]
Set-Content -Path $lcovFile -Value $null

foreach ($binary in $testBinaries) {
    $binaryName = $binary.BaseName
    $binarySummaryFile = Join-Path $summaryDir "$binaryName.txt"
    $binaryLcovFile = Join-Path $lcovDir "$binaryName.lcov"
    $binaryHtmlDir = Join-Path $htmlDir $binaryName

    Write-Host "Writing coverage summary for $binaryName"
    $reportOutput = & $llvmCovPath report $binary.FullName @commonCoverageArgs 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "llvm-cov report failed for $binaryName."
    }
    $reportOutput = Remove-CoverageNoise -Lines $reportOutput
    $reportText = ($reportOutput | Out-String).Trim()
    Set-Content -Path $binarySummaryFile -Value $reportText
    $summarySections.Add("### $binaryName`r`n$reportText") | Out-Null

    Write-Host "Writing LCOV report for $binaryName"
    $lcovOutput = & $llvmCovPath export -format=lcov $binary.FullName @commonCoverageArgs 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "llvm-cov export failed for $binaryName."
    }
    $lcovOutput = Remove-CoverageNoise -Lines $lcovOutput
    $lcovText = ($lcovOutput | Out-String).Trim()
    Set-Content -Path $binaryLcovFile -Value $lcovText
    Add-Content -Path $lcovFile -Value ($lcovText + [Environment]::NewLine)

    Write-Host "Writing HTML report for $binaryName"
    $showOutput = & $llvmCovPath show $binary.FullName @commonCoverageArgs -format=html -output-dir $binaryHtmlDir -show-line-counts-or-regions -show-branches=count 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "llvm-cov show failed for $binaryName."
    }
    $showOutput = Remove-CoverageNoise -Lines $showOutput
    $showOutput = @($showOutput | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
    if ($showOutput.Count -gt 0) {
        Write-Host ($showOutput -join [Environment]::NewLine)
    }

    $htmlIndexEntries.Add(("<li><a href='./{0}/index.html'>{0}</a></li>" -f $binaryName)) | Out-Null
}

Set-Content -Path $summaryFile -Value ($summarySections -join "`r`n`r`n")

$htmlIndex = @(
    '<!DOCTYPE html>',
    '<html lang="en">',
    '<head><meta charset="utf-8"><title>FOSSredder Coverage Reports</title></head>',
    '<body>',
    '<h1>FOSSredder Coverage Reports</h1>',
    '<p>Per-test-executable LLVM coverage reports.</p>',
    '<ul>',
    $htmlIndexEntries,
    '</ul>',
    '</body>',
    '</html>'
)
Set-Content -Path (Join-Path $htmlDir "index.html") -Value $htmlIndex

Write-Host "Coverage outputs: $resolvedOutDir"
Write-Host "Coverage summary: $summaryFile"
Write-Host "Coverage LCOV: $lcovFile"
Write-Host "Coverage HTML: $htmlDir"
