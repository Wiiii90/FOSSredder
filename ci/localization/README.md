# Localization Contract

This directory contains the CI-facing localization contract and validation
script for UI translations and bundled OCR language models.

## Contract

`localization-contract.json` is the machine-readable localization contract.
Installer and release QA use it to verify that supported UI languages and
required Tesseract OCR models are present before an installer is published.

Keep this contract in sync when a UI language, Qt translation catalog or bundled
OCR model becomes part of the product.

| File | Purpose |
|---|---|
| `localization-contract.json` | Supported UI languages and required or optional OCR models. |
| `check-localization.ps1` | Validates translation catalogs and tessdata files against the contract. |

## Validation

```powershell
.\ci\localization\check-localization.ps1
```

The script is used by CI before installer publishing so missing translations or
OCR models fail early.

