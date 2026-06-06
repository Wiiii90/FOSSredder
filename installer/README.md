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
- GitHub project, support, and release URLs
- Windows installer version metadata
- explicit close-application behavior during install/update

## Build Flow

The packaging flow is intentionally split by responsibility:

- `cmake/modules/FossredderPackaging.cmake` exposes the `package` target.
- `ci/package/package-inno.ps1` prepares staging, deploys runtime dependencies, and calls Inno Setup.
- `ci/package/validate-package.ps1` verifies that the expected installer artifact was produced.
- `installer/inno/fossredder.iss` defines the user-facing installer entry point.
- `installer/inno/includes/*.iss` split setup metadata, tasks, files, icons, and run behavior.
- `installer/assets/` owns future installer-specific branding assets.

Local package build:

```powershell
cmake --preset app
cmake --build --preset release-package
```

CI package builds are uploaded as the `fossredder-installer` artifact from the
quality workflow.

Develop package builds also update the mutable `develop-nightly` GitHub
pre-release with the latest validated installer. This nightly release is for
testing only; stable installers should be published from promoted `master`
builds as versioned releases.

## Future Professionalization

Next steps for a fully branded installer:

- add custom wizard banner and small wizard image under `installer/assets`
- extend package validation with installer metadata and optional signature checks
- add optional silent-install validation on the self-hosted runner
- publish promoted `master` installers through GitHub Releases
