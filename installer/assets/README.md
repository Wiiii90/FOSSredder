# Installer Assets

This directory owns installer-only wizard artwork.

## Required Assets

| File | Inno Setup role | Build output size |
|---|---|---|
| `wizard-image.bmp` | `WizardImageFile` | `164x314` |
| `wizard-small.bmp` | `WizardSmallImageFile` | `55x55` |

`ci/package/package-inno.ps1` reads these source images and writes normalized
24-bit BMP files with the required Inno Setup dimensions before invoking ISCC.

The application icon is owned by `app/assets/icons` and is passed to Inno Setup
as setup and uninstall metadata. Do not duplicate runtime assets here.
