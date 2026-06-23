# Roadmap

## Current baseline

The current baseline is `0.5.0`.

## Plan to 1.0.0

These feature ideas are planned before the production release:

#### Matching and import review:

- production-ready matching pipeline
- fuzzy and Levenshtein-based matching
- richer match review in the UI
- pre-import review workflow for transaction extraction
- extracted PDF text as a secondary signal for OCR improvement

#### Calculation and workflow behavior:

- tax and recoverable-cost calculation improvements
- reverse adjustments and offsets
- workflow settings for import and export
- user-facing policy hints for validation rules

#### Property allocation:

- split transaction costs and income across assigned properties
- default to equal shares when a transaction is assigned to multiple properties
- let users adjust allocation shares before calculation and export

#### Export behavior:

- stronger export generation for production use
- configurable export workflow defaults

## Post-1.0.0 expansion

These feature ideas are intentionally planned after production release:

#### Import expansion:

- scalable import support for additional bank statement templates

#### OCR modernization:

- OCR stack evaluation and modernization
- expanded OCR tuning for Poppler, OpenCV and Tesseract

#### Persistence architecture:

- dedicated persistence architecture refactor cycle
