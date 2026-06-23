# FOSSredder Installer

This directory owns the user-facing Windows installer definition for
FOSSredder.

## Purpose

The installer source lives here because setup metadata, shortcuts, wizard
artwork and Inno Setup sections belong to the delivered Windows product, not to
the CI scripts that build it.

`installer/inno/fossredder.iss` is the Inno Setup entry point. The files under
`installer/inno/includes` split the setup definition into focused sections for
metadata, languages, messages, tasks, files, icons and post-install actions.

## Structure

| Path | Purpose |
|---|---|
| `installer/inno/fossredder.iss` | Main Inno Setup script. |
| `installer/inno/includes/*.iss` | Included setup sections. |
| `installer/assets` | Installer-only wizard artwork. |

## Installer Contract

The installer is expected to create a Windows x64 setup artifact named:

```text
FOSSredder-Setup-<version>-win-x64.exe
```

It should provide:

- consistent `FOSSredder` product naming
- x64-only installation
- MIT license display during setup
- Start Menu shortcut
- optional desktop shortcut
- application icon in setup and uninstall metadata
- installer-specific wizard artwork
- bundled runtime layout produced by the package step
- explicit close-application behavior during install and update
- maintenance prompt for repair/update or uninstall when FOSSredder is already installed

## Build Entry

Local and CI packaging should build the CMake `package` target through the
configured presets:

```powershell
cmake --preset app
cmake --build --preset release-installer
```

The packaging scripts under `ci/package` prepare the staging directory, deploy
runtime dependencies and call Inno Setup. Keep detailed packaging commands and
layout validation notes in `ci/package/README.md`.

## Related Documentation

- Root build entry points: `README.md`
- Deployment design: `docs/DESIGN.md`
- Packaging scripts and validation: `ci/package/README.md`
