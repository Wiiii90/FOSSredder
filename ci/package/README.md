# Packaging (Inno Setup)

This directory contains the CI-facing packaging scripts used to build, validate
and inspect the Windows installer.

The canonical installer design and user-facing packaging policy live in
`installer/README.md`. This document is the short CI and local build reference.

## Contract

`package-layout-contract.json` is the machine-readable staged runtime contract.
Installer QA uses it to verify required executables, Qt runtime files and QML
modules after packaging.

Keep this contract in sync when a runtime file, QML module or required asset
becomes part of the installed application.

| File | Purpose |
|---|---|
| `package-layout-contract.json` | Required staged runtime layout for installer QA. |
| `test-package-layout.ps1` | Validates the staged runtime and installer output against the contract. |
| `package-inno.ps1` | Deploys runtime dependencies and invokes Inno Setup. |
| `build-installer.ps1` | Local wrapper for building the release app and installer target. |

## Prerequisites

- A Windows C++ toolchain supported by the configured CMake presets.
- A developer-local `vcpkg` installation outside the repository, configured via
  `VCPKG_ROOT` and optionally `VCPKG_INSTALLED_DIR`.
- Inno Setup 6 (`ISCC.exe`) installed.
- Qt runtime tools installed through vcpkg. Packaging uses `windeployqt`.

## Build From Visual Studio

1. Open the repository in Visual Studio (CMake project).
2. Select `Release` configuration.
3. Open the CMake Targets View.
4. Build the `package` target.

## Build From PowerShell

```powershell
cmake -DFOSSREDDER_FAST_QML_BUILD=ON --preset app
.\ci\package\build-installer.ps1
```

The script builds the app via `release-app` and then builds the installer via
`release-installer`.

## Validate The Layout

```powershell
.\ci\package\test-package-layout.ps1 -StagingDir .build\app\staging -DistDir .build\app\dist
```

## Outputs

- The `package` target installs the chosen config into `${binaryDir}/staging`.
- The packaging script deploys runtime dependencies and then calls Inno Setup to
  produce an installer under `${binaryDir}/dist`.
- The expected installer name is `FOSSredder-Setup-<version>-win-x64.exe`.
- Package logs are written under `.build\logs\package`.

## GitHub Actions

- The main `Pipeline` workflow builds and validates the installer on successful
  `develop` runs.
- The `Release` workflow builds and publishes stable installers from `v*` tags.
- Both paths run `ci/package/test-package-layout.ps1` after packaging to verify
  the staged runtime layout.

## Troubleshooting

- If ISCC is not found, ensure Inno Setup is installed and
  `C:\Program Files (x86)\Inno Setup 6\ISCC.exe` exists.
- If the installer misses DLLs, check
  `.build\logs\package\windeployqt-output.txt` and verify Qt was installed by
  vcpkg.
- Test `.build\app\staging\bin\fossredder.exe` when diagnosing staged runtime issues.
- If `cmake --preset app` resolves to the wrong `vcpkg`, verify `VCPKG_ROOT` and `VCPKG_INSTALLED_DIR` in a fresh terminal session first.
