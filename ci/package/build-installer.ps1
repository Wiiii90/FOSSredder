param(
    [string]$BuildPreset = "release-app",
    [string]$PackagePreset = "release-package",
    [string]$LogDir = ".build\\logs\\package"
)

$ErrorActionPreference = "Stop"

if (!(Test-Path $LogDir)) {
    New-Item -ItemType Directory -Path $LogDir -Force | Out-Null
}

function Invoke-LoggedNativeCommand {
    param(
        [Parameter(Mandatory)][string]$Name,
        [Parameter(Mandatory)][scriptblock]$Command,
        [Parameter(Mandatory)][string]$LogPath
    )

    Write-Host "[INFO] Running $Name (log: $LogPath)" -ForegroundColor Cyan
    & $Command 2>&1 | Tee-Object -FilePath $LogPath

    $exitCode = $LASTEXITCODE
    if ($null -eq $exitCode) {
        $exitCode = 0
    }

    if ($exitCode -ne 0) {
        Write-Host "[ERROR] $Name failed with exit code $exitCode. Last log lines:" -ForegroundColor Red
        if (Test-Path $LogPath) {
            Get-Content -Path $LogPath -Tail 80
        }
        throw "$Name failed with exit code $exitCode. See $LogPath"
    }
}

$appBuildLog = Join-Path $LogDir "app-build-output.txt"
$packageBuildLog = Join-Path $LogDir "package-build-output.txt"

Invoke-LoggedNativeCommand `
    -Name "app build preset '$BuildPreset'" `
    -LogPath $appBuildLog `
    -Command { cmake --build --preset $BuildPreset }

# Build the packaging target without MSBuild parallelism. The app is already built above,
# so this keeps the custom installer step serialized and its failures easier to diagnose.
Invoke-LoggedNativeCommand `
    -Name "package preset '$PackagePreset'" `
    -LogPath $packageBuildLog `
    -Command { cmake --build --preset $PackagePreset }
