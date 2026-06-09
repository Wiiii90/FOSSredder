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

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
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

$scriptDir = (Resolve-Path $PSScriptRoot).Path
if ($resolvedOutDir -eq $scriptDir -or $scriptDir.StartsWith($resolvedOutDir, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing to use script directory as coverage output: $resolvedOutDir"
}

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
$htmlCards = New-Object System.Collections.Generic.List[string]
Set-Content -Path $lcovFile -Value $null

function ConvertTo-HtmlText {
    param([string]$Text)

    return [System.Net.WebUtility]::HtmlEncode($Text)
}

function ConvertTo-CoverageMetric {
    param(
        [string]$Name,
        [string]$Missed,
        [string]$Total,
        [string]$Percent
    )

    $percentValue = if ($Percent -match '^\s*([0-9]+(?:\.[0-9]+)?)%?\s*$') {
        [double]::Parse($Matches[1], [System.Globalization.CultureInfo]::InvariantCulture)
    } else {
        0.0
    }

    $covered = if ($Total -match '^\d+$' -and $Missed -match '^\d+$') {
        ([int]$Total - [int]$Missed).ToString([System.Globalization.CultureInfo]::InvariantCulture)
    } else {
        "n/a"
    }

    return [pscustomobject]@{
        Name = $Name
        Covered = $covered
        Missed = $Missed
        Total = $Total
        Percent = $Percent
        PercentValue = $percentValue
    }
}

function Get-CoverageMetrics {
    param([string]$ReportText)

    $lines = @($ReportText -split "\r?\n")
    $totalLine = $lines | Where-Object { $_ -match '^\s*TOTAL\s+' } | Select-Object -Last 1
    if (!$totalLine) {
        return @()
    }

    $columns = @($totalLine.Trim() -split '\s+')
    if ($columns.Count -lt 10) {
        return @()
    }

    return @(
        ConvertTo-CoverageMetric -Name "Regions" -Missed $columns[2] -Total $columns[1] -Percent $columns[3]
        ConvertTo-CoverageMetric -Name "Functions" -Missed $columns[5] -Total $columns[4] -Percent $columns[6]
        ConvertTo-CoverageMetric -Name "Lines" -Missed $columns[8] -Total $columns[7] -Percent $columns[9]
    )
}

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

    $metrics = @(Get-CoverageMetrics -ReportText $reportText)
    $metricMarkup = if ($metrics.Count -gt 0) {
        ($metrics | ForEach-Object {
            $metricClass = if ($_.PercentValue -ge 80) { "good" } elseif ($_.PercentValue -ge 50) { "warn" } else { "low" }
            "<div class='metric $metricClass'><span>$($_.Name)</span><strong>$($_.Percent)</strong><small>$($_.Covered) / $($_.Total) covered</small></div>"
        }) -join [Environment]::NewLine
    } else {
        "<p class='muted'>No TOTAL coverage row was found for this executable.</p>"
    }

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
    $htmlCards.Add(@"
<article class="card">
  <div>
    <h2>$(ConvertTo-HtmlText $binaryName)</h2>
    <a href="./$(ConvertTo-HtmlText $binaryName)/index.html">Open detailed report</a>
  </div>
  <div class="metrics">
    $metricMarkup
  </div>
</article>
"@) | Out-Null
}

Set-Content -Path $summaryFile -Value ($summarySections -join "`r`n`r`n")

$htmlIndex = @(
    '<!DOCTYPE html>',
    '<html lang="en">',
    '<head>',
    '<meta charset="utf-8">',
    '<meta name="viewport" content="width=device-width, initial-scale=1">',
    '<title>FOSSredder Coverage Reports</title>',
    '<style>',
    ':root { color-scheme: light; --ink: #14213d; --muted: #5f6b7a; --line: #d9e2ec; --paper: #ffffff; --wash: #f5f7fb; --accent: #0f766e; --good: #15803d; --warn: #b45309; --low: #b91c1c; }',
    '* { box-sizing: border-box; }',
    'body { margin: 0; font-family: "Segoe UI", "Aptos", sans-serif; color: var(--ink); background: radial-gradient(circle at top left, #e2f5f1, transparent 34rem), linear-gradient(135deg, #f8fafc 0%, #eef2f7 100%); }',
    'main { width: min(1120px, calc(100% - 2rem)); margin: 0 auto; padding: 4rem 0; }',
    '.hero { margin-bottom: 2rem; }',
    '.eyebrow { color: var(--accent); font-size: .78rem; font-weight: 700; letter-spacing: .12em; text-transform: uppercase; }',
    'h1 { margin: .25rem 0 .5rem; font-size: clamp(2.2rem, 5vw, 4rem); line-height: 1; }',
    '.lead { max-width: 48rem; color: var(--muted); font-size: 1.05rem; }',
    '.grid { display: grid; gap: 1rem; }',
    '.card { display: grid; grid-template-columns: minmax(12rem, 1fr) 2fr; gap: 1rem; align-items: center; padding: 1.1rem; border: 1px solid var(--line); border-radius: 1.25rem; background: rgba(255,255,255,.82); box-shadow: 0 16px 44px rgba(15, 23, 42, .08); backdrop-filter: blur(8px); }',
    '.card h2 { margin: 0 0 .35rem; font-size: 1.05rem; }',
    'a { color: var(--accent); font-weight: 700; text-decoration: none; }',
    'a:hover { text-decoration: underline; }',
    '.metrics { display: grid; grid-template-columns: repeat(3, minmax(0, 1fr)); gap: .75rem; }',
    '.metric { border-radius: .9rem; padding: .75rem; background: var(--wash); border: 1px solid transparent; }',
    '.metric span, .metric small { display: block; color: var(--muted); }',
    '.metric span { font-size: .75rem; font-weight: 700; letter-spacing: .08em; text-transform: uppercase; }',
    '.metric strong { display: block; margin: .25rem 0; font-size: 1.45rem; }',
    '.metric.good { border-color: rgba(21,128,61,.22); }',
    '.metric.warn { border-color: rgba(180,83,9,.25); }',
    '.metric.low { border-color: rgba(185,28,28,.25); }',
    '.metric.good strong { color: var(--good); }',
    '.metric.warn strong { color: var(--warn); }',
    '.metric.low strong { color: var(--low); }',
    '.muted { color: var(--muted); }',
    '@media (max-width: 760px) { .card, .metrics { grid-template-columns: 1fr; } main { padding: 2rem 0; } }',
    '</style>',
    '</head>',
    '<body>',
    '<main>',
    '<section class="hero">',
    '<div class="eyebrow">LLVM source coverage</div>',
    '<h1>FOSSredder Coverage Reports</h1>',
    '<p class="lead">Per-test-executable coverage generated from LLVM instrumentation. The README badge is calculated by Codecov from the combined LCOV upload, while this page links to the detailed HTML reports produced during CI.</p>',
    '</section>',
    '<section class="grid">',
    $htmlCards,
    '</section>',
    '</main>',
    '</body>',
    '</html>'
)
Set-Content -Path (Join-Path $htmlDir "index.html") -Value $htmlIndex

Write-Host "Coverage outputs: $resolvedOutDir"
Write-Host "Coverage summary: $summaryFile"
Write-Host "Coverage LCOV: $lcovFile"
Write-Host "Coverage HTML: $htmlDir"
