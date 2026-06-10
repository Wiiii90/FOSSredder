# Packaging (Inno Setup) - Local Guide

This document describes how to create a Windows `setup.exe` locally using the repository's packaging scripts and the `package` CMake target.

The canonical installer contract lives in `installer/README.md`. Keep this file
as a short CI/local command reference.

The runtime layout contract used by Installer QA lives in
`ci/package/package-layout-contract.json`.

Prerequisites
- Visual Studio 2026 (VS18) with "Desktop development with C++" workload.
- Inno Setup (`ISCC.exe`) installed (we recommend installing via Chocolatey).
- A developer-local `vcpkg` installation outside the repository, configured via `VCPKG_ROOT` and optionally `VCPKG_INSTALLED_DIR` as documented in the root `README.md`.
- Qt via vcpkg (this repo uses `windeployqt` during packaging).

Steps (Visual Studio UI - recommended)
1. Open the repository in Visual Studio (CMake project).
2. Select `Release` configuration.
3. Open the CMake Targets View.
4. Build the `package` target.

What happens
- The `package` target installs the chosen config into `${binaryDir}/staging`.
- The packaging script deploys runtime dependencies and then calls Inno Setup to produce an installer under `${binaryDir}/dist`.
- The expected installer name is `FOSSredder-Setup-<version>-win-x64.exe`.
- Installer and release workflows run `ci/package/test-package-layout.ps1` after packaging to verify the staged runtime layout.

Steps (Command line)

```powershell
# Configure + build the same fast QML package path used by CI
cmake -DFOSSREDDER_FAST_QML_BUILD=ON --preset app
.\ci\package\build-installer.ps1

# Validate staged runtime layout
.\ci\package\test-package-layout.ps1 -StagingDir .build\app\staging -DistDir .build\app\dist
```

Manual GitHub Actions loop
- Use the `Installer` workflow from the Actions tab when you only need to test
  installer packaging and layout validation.
- Use the main `Pipeline` workflow for full release-readiness validation.
- Successful `develop` pipeline runs trigger the `Installer` workflow to update
  the mutable `develop-nightly` pre-release.

Troubleshooting
- If ISCC is not found, ensure Inno Setup is installed and `C:\Program Files (x86)\Inno Setup 6\ISCC.exe` exists.
- If the installer misses DLLs, check `.build\logs\package\windeployqt-output.txt` and verify Qt was installed by vcpkg.
- Test `staging\bin\fossredder.exe` before building the installer.
- If `cmake --preset app` resolves to the wrong `vcpkg`, verify `VCPKG_ROOT` and `VCPKG_INSTALLED_DIR` in a fresh terminal session first.
