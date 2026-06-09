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
$detailLinks = New-Object System.Collections.Generic.List[string]
$htmlRows = New-Object System.Collections.Generic.List[string]
$coverageRows = New-Object System.Collections.Generic.List[object]
Set-Content -Path $lcovFile -Value $null

function ConvertTo-HtmlText {
    param([string]$Text)

    return [System.Net.WebUtility]::HtmlEncode($Text)
}

function Format-CoveragePercent {
    param([double]$Value)

    return ("{0:N1}%" -f $Value)
}

function Get-CoverageTone {
    param([double]$Value)

    if ($Value -ge 90) { return "excellent" }
    if ($Value -ge 75) { return "good" }
    if ($Value -ge 60) { return "fair" }
    if ($Value -ge 30) { return "warn" }
    return "low"
}

function Get-CoveragePercentFromCounts {
    param(
        [double]$Covered,
        [double]$Total
    )

    if ($Total -le 0) {
        return 0.0
    }

    return ($Covered / $Total) * 100.0
}

function New-CoveragePill {
    param(
        [double]$Value,
        [string]$Label
    )

    $tone = Get-CoverageTone -Value $Value
    $display = Format-CoveragePercent -Value $Value
    $safeLabel = ConvertTo-HtmlText $Label
    $barWidth = [Math]::Max(0, [Math]::Min(100, $Value))

    return @"
<div class="coverage-pill $tone" aria-label="$safeLabel $display">
  <div class="coverage-pill__top"><span>$safeLabel</span><strong>$display</strong></div>
  <div class="coverage-bar"><span style="width: $barWidth%"></span></div>
</div>
"@
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

function Get-LcovLineCoverage {
    param([string]$Path)

    if (!(Test-Path $Path)) {
        return $null
    }

    $linesHit = 0.0
    $linesFound = 0.0

    foreach ($line in Get-Content -Path $Path) {
        if ($line -match '^LH:(\d+)$') {
            $linesHit += [double]$Matches[1]
            continue
        }

        if ($line -match '^LF:(\d+)$') {
            $linesFound += [double]$Matches[1]
        }
    }

    if ($linesFound -le 0) {
        return $null
    }

    $percent = Get-CoveragePercentFromCounts -Covered $linesHit -Total $linesFound
    return [pscustomobject]@{
        Hit = $linesHit
        Found = $linesFound
        Percent = $percent
    }
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
    if ($metrics.Count -gt 0) {
        $regionsMetric = $metrics | Where-Object { $_.Name -eq "Regions" } | Select-Object -First 1
        $functionsMetric = $metrics | Where-Object { $_.Name -eq "Functions" } | Select-Object -First 1
        $linesMetric = $metrics | Where-Object { $_.Name -eq "Lines" } | Select-Object -First 1
        $rowAverage = @($regionsMetric.PercentValue, $functionsMetric.PercentValue, $linesMetric.PercentValue) |
            Measure-Object -Average |
            Select-Object -ExpandProperty Average

        $coverageRows.Add([pscustomobject]@{
            Name = $binaryName
            Regions = $regionsMetric.PercentValue
            RegionsCovered = [double]$regionsMetric.Covered
            RegionsTotal = [double]$regionsMetric.Total
            Functions = $functionsMetric.PercentValue
            FunctionsCovered = [double]$functionsMetric.Covered
            FunctionsTotal = [double]$functionsMetric.Total
            Lines = $linesMetric.PercentValue
            LinesCovered = [double]$linesMetric.Covered
            LinesTotal = [double]$linesMetric.Total
            Average = $rowAverage
        }) | Out-Null

        $htmlRows.Add(@"
<tr>
  <th scope="row"><a href="./$(ConvertTo-HtmlText $binaryName)/index.html">$(ConvertTo-HtmlText $binaryName)</a></th>
  <td>$(New-CoveragePill -Value $regionsMetric.PercentValue -Label "Regions")</td>
  <td>$(New-CoveragePill -Value $functionsMetric.PercentValue -Label "Functions")</td>
  <td>$(New-CoveragePill -Value $linesMetric.PercentValue -Label "Lines")</td>
  <td>$(New-CoveragePill -Value $rowAverage -Label "Average")</td>
</tr>
"@) | Out-Null
    }

    $detailLinks.Add(@"
<li><a href="./$(ConvertTo-HtmlText $binaryName)/index.html">$(ConvertTo-HtmlText $binaryName)</a></li>
"@) | Out-Null

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

}

Set-Content -Path $summaryFile -Value ($summarySections -join "`r`n`r`n")

$lcovLineCoverage = Get-LcovLineCoverage -Path $lcovFile
$lcovHeroCard = ""
if ($lcovLineCoverage) {
    $lcovHeroCard = @"
<aside class="hero-card">
  <span>LCOV line coverage</span>
  <strong>$(Format-CoveragePercent -Value $lcovLineCoverage.Percent)</strong>
  <div class="coverage-bar"><span style="width: $([Math]::Max(0, [Math]::Min(100, $lcovLineCoverage.Percent)))%"></span></div>
  <small>$([int]$lcovLineCoverage.Hit) / $([int]$lcovLineCoverage.Found) lines hit. This is the closest local counterpart to the Codecov badge.</small>
</aside>
"@
}

$summaryTable = "<p class='muted'>No summary metrics were found.</p>"
if ($coverageRows.Count -gt 0) {
    $regionsAverage = ($coverageRows | Measure-Object -Property Regions -Average).Average
    $functionsAverage = ($coverageRows | Measure-Object -Property Functions -Average).Average
    $linesAverage = ($coverageRows | Measure-Object -Property Lines -Average).Average
    $overallAverage = ($coverageRows | Measure-Object -Property Average -Average).Average
    $regionsWeighted = Get-CoveragePercentFromCounts `
        -Covered (($coverageRows | Measure-Object -Property RegionsCovered -Sum).Sum) `
        -Total (($coverageRows | Measure-Object -Property RegionsTotal -Sum).Sum)
    $functionsWeighted = Get-CoveragePercentFromCounts `
        -Covered (($coverageRows | Measure-Object -Property FunctionsCovered -Sum).Sum) `
        -Total (($coverageRows | Measure-Object -Property FunctionsTotal -Sum).Sum)
    $linesWeighted = Get-CoveragePercentFromCounts `
        -Covered (($coverageRows | Measure-Object -Property LinesCovered -Sum).Sum) `
        -Total (($coverageRows | Measure-Object -Property LinesTotal -Sum).Sum)
    $weightedAverage = @($regionsWeighted, $functionsWeighted, $linesWeighted) |
        Measure-Object -Average |
        Select-Object -ExpandProperty Average

    $summaryTable = @"
<section class="summary">
  <div class="section-heading">
    <div>
      <span class="section-kicker">Overview</span>
      <h2>Coverage Matrix</h2>
    </div>
    <p>Matrix values come from LLVM report totals per test executable. The LCOV line coverage above is the closest local counterpart to the Codecov badge.</p>
  </div>
  <div class="table-wrap">
    <table>
      <thead>
        <tr>
          <th scope="col">Report</th>
          <th scope="col">Regions</th>
          <th scope="col">Functions</th>
          <th scope="col">Lines</th>
          <th scope="col">Average</th>
        </tr>
      </thead>
      <tbody>
        $($htmlRows -join [Environment]::NewLine)
      </tbody>
      <tfoot>
        <tr class="weighted-total">
          <th scope="row">Weighted total</th>
          <td>$(New-CoveragePill -Value $regionsWeighted -Label "Regions")</td>
          <td>$(New-CoveragePill -Value $functionsWeighted -Label "Functions")</td>
          <td>$(New-CoveragePill -Value $linesWeighted -Label "Lines")</td>
          <td>$(New-CoveragePill -Value $weightedAverage -Label "Average")</td>
        </tr>
        <tr>
          <th scope="row">Column average</th>
          <td>$(New-CoveragePill -Value $regionsAverage -Label "Regions")</td>
          <td>$(New-CoveragePill -Value $functionsAverage -Label "Functions")</td>
          <td>$(New-CoveragePill -Value $linesAverage -Label "Lines")</td>
          <td>$(New-CoveragePill -Value $overallAverage -Label "Average")</td>
        </tr>
      </tfoot>
    </table>
  </div>
</section>
"@
}

$htmlIndex = @(
    '<!DOCTYPE html>',
    '<html lang="en">',
    '<head>',
    '<meta charset="utf-8">',
    '<meta name="viewport" content="width=device-width, initial-scale=1">',
    '<title>FOSSredder Coverage Reports</title>',
    '<style>',
    ':root { color-scheme: light; --ink: #14213d; --muted: #5f6b7a; --line: #d9e2ec; --paper: #ffffff; --wash: #f5f7fb; --accent: #0f766e; --excellent: #047857; --good: #16a34a; --fair: #84cc16; --warn: #f59e0b; --low: #ef4444; }',
    '* { box-sizing: border-box; }',
    'body { margin: 0; font-family: "Segoe UI", "Aptos", sans-serif; color: var(--ink); background: radial-gradient(circle at top left, #e2f5f1, transparent 34rem), linear-gradient(135deg, #f8fafc 0%, #eef2f7 100%); }',
    'main { width: min(1240px, calc(100% - 2rem)); margin: 0 auto; padding: 4rem 0; }',
    '.hero { display: grid; grid-template-columns: minmax(0, 1fr) auto; gap: 1.5rem; align-items: end; margin-bottom: 2rem; }',
    '.eyebrow { color: var(--accent); font-size: .78rem; font-weight: 700; letter-spacing: .12em; text-transform: uppercase; }',
    'h1 { margin: .25rem 0 .5rem; font-size: clamp(2.2rem, 5vw, 4rem); line-height: 1; }',
    'h2 { margin: 0 0 1rem; }',
    '.lead { max-width: 48rem; color: var(--muted); font-size: 1.05rem; }',
    '.hero-card { min-width: 18rem; padding: 1rem; border: 1px solid rgba(15, 118, 110, .18); border-radius: 1.15rem; background: rgba(255,255,255,.78); box-shadow: 0 16px 44px rgba(15, 23, 42, .08); }',
    '.hero-card span, .hero-card small { display: block; color: var(--muted); }',
    '.hero-card span { font-size: .72rem; font-weight: 800; letter-spacing: .12em; text-transform: uppercase; }',
    '.hero-card strong { display: block; margin: .25rem 0 .45rem; color: var(--accent); font-size: 2.25rem; line-height: 1; }',
    '.hero-card small { margin-top: .55rem; line-height: 1.45; }',
    '.summary { margin: 2rem 0; padding: 1.25rem; border: 1px solid var(--line); border-radius: 1.35rem; background: rgba(255,255,255,.86); box-shadow: 0 16px 44px rgba(15, 23, 42, .08); backdrop-filter: blur(8px); }',
    '.section-heading { display: flex; justify-content: space-between; gap: 1rem; align-items: end; margin-bottom: 1rem; }',
    '.section-heading h2 { margin: .1rem 0 0; }',
    '.section-heading p { max-width: 34rem; margin: 0; color: var(--muted); font-size: .95rem; }',
    '.section-kicker { color: var(--accent); font-size: .72rem; font-weight: 800; letter-spacing: .12em; text-transform: uppercase; }',
    '.table-wrap { overflow-x: auto; }',
    'table { width: 100%; border-collapse: collapse; min-width: 820px; }',
    'th, td { padding: .7rem; border-bottom: 1px solid var(--line); vertical-align: middle; text-align: left; }',
    'thead th, tfoot th, tfoot td { background: rgba(241, 245, 249, .8); }',
    '.weighted-total th, .weighted-total td { background: rgba(220, 252, 231, .65); }',
    'tbody th { font-size: .9rem; }',
    'a { color: var(--accent); font-weight: 700; text-decoration: none; }',
    'a:hover { text-decoration: underline; }',
    '.details { margin-top: 1.5rem; padding: 1rem 1.25rem; border: 1px solid var(--line); border-radius: 1.15rem; background: rgba(255,255,255,.68); }',
    '.details h2 { margin: 0 0 .35rem; font-size: 1.1rem; }',
    '.details p { margin: 0 0 .85rem; color: var(--muted); }',
    '.detail-list { display: flex; flex-wrap: wrap; gap: .55rem; padding: 0; margin: 0; list-style: none; }',
    '.detail-list a { display: inline-flex; padding: .45rem .7rem; border: 1px solid var(--line); border-radius: 999px; background: var(--paper); box-shadow: 0 8px 18px rgba(15, 23, 42, .06); }',
    '.coverage-pill { min-width: 10rem; }',
    '.coverage-pill__top { display: flex; justify-content: space-between; gap: .75rem; margin-bottom: .35rem; }',
    '.coverage-pill__top span { color: var(--muted); font-size: .72rem; font-weight: 700; letter-spacing: .08em; text-transform: uppercase; }',
    '.coverage-pill__top strong { font-size: .95rem; }',
    '.coverage-bar { height: .5rem; overflow: hidden; border-radius: 999px; background: #e7edf3; box-shadow: inset 0 1px 2px rgba(15, 23, 42, .08); }',
    '.coverage-bar span, .coverage-bar i { display: block; height: 100%; border-radius: inherit; background: var(--bar); }',
    '.excellent { --bar: linear-gradient(90deg, #047857, #10b981); }',
    '.good { --bar: linear-gradient(90deg, #16a34a, #86efac); }',
    '.fair { --bar: linear-gradient(90deg, #84cc16, #d9f99d); }',
    '.warn { --bar: linear-gradient(90deg, #f59e0b, #fde68a); }',
    '.low { --bar: linear-gradient(90deg, #ef4444, #fecaca); }',
    '.coverage-pill.excellent strong { color: var(--excellent); }',
    '.coverage-pill.good strong { color: var(--good); }',
    '.coverage-pill.fair strong { color: #4d7c0f; }',
    '.coverage-pill.warn strong { color: #b45309; }',
    '.coverage-pill.low strong { color: #b91c1c; }',
    '.muted { color: var(--muted); }',
    '@media (max-width: 760px) { .hero, .section-heading { display: block; } main { padding: 2rem 0; } }',
    '</style>',
    '</head>',
    '<body>',
    '<main>',
    '<section class="hero">',
    '<div>',
    '<div class="eyebrow">LLVM source coverage</div>',
    '<h1>FOSSredder Coverage</h1>',
    '<p class="lead">Coverage generated from LLVM instrumentation across the Windows CI test executables. The LCOV line coverage is calculated from the combined report uploaded to Codecov.</p>',
    '</div>',
    $lcovHeroCard,
    '</section>',
    $summaryTable,
    '<section class="details">',
    '<h2>Detailed Reports</h2>',
    '<p>Open the generated LLVM HTML report for a specific test executable.</p>',
    '<ul class="detail-list">',
    $detailLinks,
    '</ul>',
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
