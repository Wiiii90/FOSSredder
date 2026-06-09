# FOSSredder Installer

This directory owns the Windows installer definition and release packaging
expectations for FOSSredder.

## Installer Contract

The installer is built with Inno Setup from `installer/inno/fossredder.iss`.
It packages the staged application produced by CMake install and creates a
Windows x64 installer artifact:

```text
FOSSredder-Setup-<version>-win-x64.exe
```

The installer should provide:

- consistent `FOSSredder` product naming
- x64-only installation
- MIT license display during setup
- Start Menu shortcut
- optional desktop shortcut
- application icon in setup and uninstall metadata
- bundled Tesseract OCR models required for local statement import
- GitHub project, support, and release URLs
- Windows installer version metadata
- explicit close-application behavior during install/update

## Build Flow

The packaging flow is intentionally split by responsibility:

- `cmake/modules/FossredderPackaging.cmake` exposes the `package` target.
- `ci/package/package-inno.ps1` prepares staging, deploys runtime dependencies, and calls Inno Setup.
- `ci/package/validate-package.ps1` verifies that the expected installer artifact was produced.
- `ci/package/test-package-layout.ps1` verifies the staged runtime contract used by the installer.
- `ci/localization/localization-contract.json` defines supported UI languages and bundled OCR model expectations.
- `installer/inno/fossredder.iss` defines the user-facing installer entry point.
- `installer/inno/includes/*.iss` split setup metadata, tasks, files, icons, and run behavior.
- `infra/text-recognition/res/tessdata` provides the bundled OCR models installed to `bin/res/tessdata`.
- `installer/assets/` owns installer-specific branding assets if setup artwork diverges from the application icon.

Local package build:

```powershell
cmake --preset app
cmake --build --preset release-package
```

CI package builds are uploaded as the `fossredder-installer` artifact from the
pipeline workflow.

Develop package builds also update the mutable `develop-nightly` GitHub
pre-release with the latest validated installer. This nightly release is for
testing only; stable installers should be published from promoted `master`
builds as versioned releases.

## Release Hardening

The v0.5.0 installer is expected to install a self-contained Windows runtime for
the application. Release validation should confirm:

- the installer contains `bin/fossredder.exe`
- Qt runtime DLLs, plugins, and QML imports are present under `bin`
- Tesseract model files are present under `bin/res/tessdata`
- compiled Qt translation catalogs are present under `bin/i18n`
- the installed application launches from the Start Menu shortcut
- PDF import can run without requiring a manually configured external
  Tesseract data path
- promoted `master` installers are published through GitHub Releases
