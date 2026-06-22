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
    if ($Value -ge 50) { return "fair" }
    if ($Value -ge 25) { return "warn" }
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

function New-CoverageCell {
    param(
        [double]$Value
    )

    $tone = Get-CoverageTone -Value $Value
    $display = Format-CoveragePercent -Value $Value

    return @"
<span class="coverage-value $tone">$display</span>
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
  <td>$(New-CoverageCell -Value $regionsMetric.PercentValue)</td>
  <td>$(New-CoverageCell -Value $functionsMetric.PercentValue)</td>
  <td>$(New-CoverageCell -Value $linesMetric.PercentValue)</td>
  <td>$(New-CoverageCell -Value $rowAverage)</td>
</tr>
"@) | Out-Null
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

}

Set-Content -Path $summaryFile -Value ($summarySections -join "`r`n`r`n")

$lcovLineCoverage = Get-LcovLineCoverage -Path $lcovFile

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
    $lcovTotalRow = ""
    if ($lcovLineCoverage) {
        $lcovTotalRow = @"
        <tr class="codecov-total">
          <th scope="row">Codecov input (LCOV)</th>
          <td class="muted">-</td>
          <td class="muted">-</td>
          <td>$(New-CoverageCell -Value $lcovLineCoverage.Percent)</td>
          <td class="muted">$([int]$lcovLineCoverage.Hit) / $([int]$lcovLineCoverage.Found) lines</td>
        </tr>
"@
    }

    $summaryTable = @"
<section class="summary">
  <div class="section-heading">
    <div>
      <span class="section-kicker">LLVM coverage</span>
      <h2>Coverage Matrix</h2>
    </div>
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
          <td>$(New-CoverageCell -Value $regionsWeighted)</td>
          <td>$(New-CoverageCell -Value $functionsWeighted)</td>
          <td>$(New-CoverageCell -Value $linesWeighted)</td>
          <td>$(New-CoverageCell -Value $weightedAverage)</td>
        </tr>
        $lcovTotalRow
        <tr>
          <th scope="row">Column average</th>
          <td>$(New-CoverageCell -Value $regionsAverage)</td>
          <td>$(New-CoverageCell -Value $functionsAverage)</td>
          <td>$(New-CoverageCell -Value $linesAverage)</td>
          <td>$(New-CoverageCell -Value $overallAverage)</td>
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
    ':root { color-scheme: light; --ink: #14213d; --muted: #64748b; --line: #d8e0ea; --paper: #ffffff; --wash: #f6f8fb; --accent: #0f766e; --excellent: #047857; --good: #15803d; --fair: #4d7c0f; --warn: #b45309; --low: #b91c1c; }',
    '* { box-sizing: border-box; }',
    'body { margin: 0; font-family: "Segoe UI", "Aptos", sans-serif; color: var(--ink); background: #f6f8fb; }',
    'main { width: min(1180px, calc(100% - 2rem)); margin: 0 auto; padding: 3rem 0; }',
    '.hero { margin-bottom: 1.5rem; }',
    '.eyebrow { color: var(--accent); font-size: .75rem; font-weight: 800; letter-spacing: .14em; text-transform: uppercase; }',
    'h1 { margin: .25rem 0 .4rem; font-size: clamp(2rem, 4vw, 3.35rem); line-height: 1; }',
    'h2 { margin: 0 0 1rem; }',
    '.lead { max-width: 56rem; color: var(--muted); font-size: 1.02rem; }',
    '.summary { margin: 1.5rem 0; padding: 1.25rem; border: 1px solid var(--line); border-radius: .9rem; background: var(--paper); box-shadow: 0 10px 28px rgba(15, 23, 42, .05); }',
    '.section-heading { display: flex; justify-content: space-between; gap: 1rem; align-items: end; margin-bottom: .9rem; }',
    '.section-heading h2 { margin: .1rem 0 0; }',
    '.section-kicker { color: var(--accent); font-size: .72rem; font-weight: 800; letter-spacing: .12em; text-transform: uppercase; }',
    '.table-wrap { overflow-x: auto; }',
    'table { width: 100%; border-collapse: collapse; min-width: 760px; }',
    'th, td { padding: .68rem .75rem; border-bottom: 1px solid var(--line); vertical-align: middle; text-align: right; font-variant-numeric: tabular-nums; }',
    'th:first-child, td:first-child { text-align: left; }',
    'thead th, tfoot th, tfoot td { background: #f1f5f9; }',
    '.weighted-total th, .weighted-total td { background: #ecfdf5; }',
    '.codecov-total th, .codecov-total td { background: #eff6ff; }',
    'tbody th { font-size: .9rem; }',
    'a { color: var(--accent); font-weight: 700; text-decoration: none; }',
    'a:hover { text-decoration: underline; }',
    '.coverage-value { display: inline-flex; min-width: 4.8rem; justify-content: flex-end; font-weight: 800; }',
    '.coverage-value.excellent { color: var(--excellent); }',
    '.coverage-value.good { color: var(--good); }',
    '.coverage-value.fair { color: var(--fair); }',
    '.coverage-value.warn { color: var(--warn); }',
    '.coverage-value.low { color: var(--low); }',
    '.muted { color: var(--muted); }',
    '@media (max-width: 760px) { .section-heading { display: block; } main { padding: 2rem 0; } }',
    '</style>',
    '</head>',
    '<body>',
    '<main>',
    '<section class="hero">',
    '<div>',
    '<div class="eyebrow">LLVM source coverage</div>',
    '<h1>FOSSredder Coverage</h1>',
    '<p class="lead">LLVM coverage report for the Windows CI test executables.</p>',
    '</div>',
    '</section>',
    $summaryTable,
    '</main>',
    '</body>',
    '</html>'
)
Set-Content -Path (Join-Path $htmlDir "index.html") -Value $htmlIndex

Write-Host "Coverage outputs: $resolvedOutDir"
Write-Host "Coverage summary: $summaryFile"
Write-Host "Coverage LCOV: $lcovFile"
Write-Host "Coverage HTML: $htmlDir"
