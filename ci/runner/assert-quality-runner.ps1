# Shared self-hosted runner prerequisite checks for CI workflows.
param(
    [switch]$RequireClangTidy,
    [switch]$RequireCoverageTools,
    [switch]$RequireInnoSetup,
    [switch]$RequireDoxygen,
    [switch]$RequireGitHubCli,
    [switch]$SkipBuildPrerequisites
)

function Resolve-VcpkgRoot {
    if (-not [string]::IsNullOrWhiteSpace($env:VCPKG_ROOT) -and (Test-Path $env:VCPKG_ROOT)) {
        return $env:VCPKG_ROOT
    }

    $userRoot = [Environment]::GetEnvironmentVariable('VCPKG_ROOT', 'User')
    if (-not [string]::IsNullOrWhiteSpace($userRoot) -and (Test-Path $userRoot)) {
        return $userRoot
    }

    $machineRoot = [Environment]::GetEnvironmentVariable('VCPKG_ROOT', 'Machine')
    if (-not [string]::IsNullOrWhiteSpace($machineRoot) -and (Test-Path $machineRoot)) {
        return $machineRoot
    }

    if (Test-Path 'P:\vcpkg') {
        return 'P:\vcpkg'
    }

    return $null
}

$minimumCmakeVersion = [Version]'4.0.1'
$cmakeVersion = $null

if (-not $SkipBuildPrerequisites) {
    $cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmakeCommand) {
        throw "cmake was not found on the runner. Install CMake 4.0.1 or newer on the self-hosted Windows runner."
    }

    $cmakeVersionLine = (& $cmakeCommand.Path --version | Select-Object -First 1)
    if ($cmakeVersionLine -notmatch 'cmake version ([0-9]+\.[0-9]+\.[0-9]+)') {
        throw "Could not determine CMake version. Output: $cmakeVersionLine"
    }

    $cmakeVersion = [Version]$Matches[1]
    if ($cmakeVersion -lt $minimumCmakeVersion) {
        throw "CMake $cmakeVersion is too old. Minimum required is $minimumCmakeVersion."
    }

    # Always resolve VCPKG_ROOT with user-preference so Visual Studio's process-level vcpkg does not override the intended user install
    $resolvedVcpkg = Resolve-VcpkgRoot
    if (-not $resolvedVcpkg) {
        throw "VCPKG_ROOT could not be resolved (check user or machine environment variables or P:\\vcpkg)."
    }

    # Export resolved path into the current process so subsequent steps use the correct vcpkg
    $env:VCPKG_ROOT = $resolvedVcpkg

    $toolchainFile = Join-Path $env:VCPKG_ROOT 'scripts\buildsystems\vcpkg.cmake'
    if (!(Test-Path $toolchainFile)) {
        throw "The vcpkg toolchain was not found at '$toolchainFile'. Check VCPKG_ROOT on the self-hosted runner."
    }
}

if ($RequireClangTidy -and -not (Get-Command clang-tidy -ErrorAction SilentlyContinue)) {
    throw "clang-tidy was not found on the self-hosted runner. Install LLVM/clang-tidy on the runner."
}

if ($RequireCoverageTools) {
    foreach ($tool in @('clang-cl.exe', 'llvm-profdata.exe', 'llvm-cov.exe')) {
        if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
            throw "$tool was not found on the self-hosted runner. Install the LLVM toolset on the runner."
        }
    }
}

if ($RequireInnoSetup) {
    $isccPath = 'C:\Program Files (x86)\Inno Setup 6\ISCC.exe'
    if (!(Test-Path $isccPath)) {
        throw "Inno Setup compiler was not found at '$isccPath'. Install Inno Setup 6 on the self-hosted runner."
    }
}

if ($RequireDoxygen -and -not (Get-Command doxygen -ErrorAction SilentlyContinue)) {
    throw "doxygen was not found on the self-hosted runner. Install Doxygen on the runner."
}

if ($RequireGitHubCli -and -not (Get-Command gh -ErrorAction SilentlyContinue)) {
    throw "GitHub CLI 'gh' was not found on the self-hosted runner. Install GitHub CLI on the runner."
}

Write-Host "Runner prerequisites ok."
if (-not $SkipBuildPrerequisites) {
    Write-Host "Using cmake: $cmakeVersion"
    Write-Host "Using VCPKG_ROOT: $env:VCPKG_ROOT"
}
