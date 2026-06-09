# Installer Assets

This directory owns installer-specific branding assets when they differ from
the application assets.

The v0.5.0 installer uses the canonical application icon from
`app/assets/icons/fossredder.ico` for setup metadata, uninstall metadata, and
Windows shell presentation. It also uses installer-specific wizard artwork from
this directory:

- `wizard-banner.bmp`
- `wizard-small.bmp`

`ci/package/package-inno.ps1` normalizes these source images into Inno Setup
BMP assets with the required dimensions before invoking ISCC.

Runtime assets are installed from their owning project modules instead of being
duplicated here.

Important runtime assets:

- Tesseract OCR models are installed from `infra/text-recognition/res/tessdata`
  to `bin/res/tessdata`.
- QML runtime sources are installed from `ui/qml` to `bin/qml`.
- Qt plugins and runtime DLLs are deployed by `ci/package/package-inno.ps1`.

Keep this directory for installer-only artwork when setup branding differs from
the application branding.
