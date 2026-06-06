# FossRedder Architecture Target

## Goal

- `core` owns domain rules, application workflows, and ports
- `infra` owns technical adapters and external library integration
- `api/` stays removed
- `services/` stays retired as a module name

## Target Layout

```text
app/
core/
infra/
  archive/
  image-processing/
  pdf-rendering/
  text-recognition/
persistence/
ui/
debug/
docs/
installer/
ci/
cmake/
```

## Core Modules

### `core/domain`

- entities
- value objects
- domain policies
- domain invariants

### `core/application`

- import use cases
- export use cases
- analysis use cases
- workspace orchestration
- storage orchestration

### `core/ports`

Ports are grouped by boundary. Keep each port family small and focused.

#### `core/ports/presenters`

- presenter interfaces
- output formatting contracts

#### `core/ports/repositories`

- repository interfaces
- persistence-facing abstractions

#### `core/ports/storage`

- storage interfaces
- file and blob access abstractions

#### `core/ports/image-processing`

- `IImageProcessor`
- image processing ports

#### `core/ports/pdf-rendering`

- `IPdfRenderer`
- PDF rendering ports

#### `core/ports/text-recognition`

- `ITextRecognizer`
- OCR and text recognition ports

#### `core/ports/archive`

- `IArchive`
- archive packaging ports

Compatibility aliases may remain during the migration slice, but new code should depend on the capability-oriented names above.

### `core/utils`

- timestamp helpers
- id generation helpers
- string helpers

## Infra Modules

### `infra/archive`

- ZIP archive creation for export packaging
- file collection and packaging only

### `infra/image-processing`

- image denoising
- masking
- detection
- cropping

### `infra/pdf-rendering`

- PDF rendering
- PDF metadata extraction

### `infra/text-recognition`

- OCR extraction
- table text reconstruction

## Module Naming Convention

- name infra modules by capability, not by vendor
- use hyphenated names for multi-word capabilities
- prefer `image-processing`, `pdf-rendering`, `text-recognition`, `archive`
- keep `archive` as the short canonical name unless the team explicitly wants the more verbose `archive-creation`
- keep vendor names in adapter classes, internal helpers, and dependency declarations only

## Naming Rules

### Ports

- name by responsibility or capability, not by vendor
- group ports by boundary: `presenters`, `repositories`, `storage`, and one folder per capability
- prefer `IImageProcessor`, `IPdfRenderer`, `ITextRecognizer`, `IArchive`
- keep vendor names out of the core API unless the capability itself is vendor-specific
- avoid generic catch-all folders for request/result/types if a feature-specific home exists

### Public adapter entry points

- name by backend plus capability
- prefer `OpenCvImageProcessorAdapter`
- prefer `PopplerPdfRendererAdapter`
- prefer `TesseractTextRecognizerAdapter`
- prefer `ZipArchiveAdapter`

### Internal helpers

- keep them neutral and local to the module
- use names like `PopplerCore` or `TesseractCore` only for private implementation helpers
- avoid `service` / `engine` as architectural layers

### File Names

- public headers should match the capability or adapter name
- source files should match the public class they implement
- temporary compatibility wrappers are allowed, but they should be removed in a later cleanup slice

### Folder Names

- keep folder names stable and easy to scan
- prefer `presenters`, `repositories`, `storage`, and single-purpose capability folders like `image-processing`, `pdf-rendering`, `text-recognition`, `archive`
- put DTOs and type headers next to the feature they belong to
- do not create a shared top-level `types` dump unless the types are truly cross-cutting

## Use Case Split

### Import

Keep orchestration in `core/application/import`.

Technical steps belong to `infra`:

- PDF rendering
- OCR
- image masking
- image detection
- cropping

### Export

Keep orchestration in `core/application/export`.

Technical steps belong to `infra`:

- archive packaging

## CMake Targets

Target naming should eventually be capability-based and uniform:

- `FossRedder::Core`
- `FossRedder::Persistence`
- `FossRedder::InfraArchive`
- `FossRedder::InfraImageProcessing`
- `FossRedder::InfraPdfRendering`
- `FossRedder::InfraTextRecognition`
- `FossRedder::Debug`
- `FossRedder::Ui`
- `fossredder`

Later, the four infra targets may be folded behind an umbrella `FossRedder::Infra` if that simplifies consumption.

## Dependency Rules

- `app` depends on `ui`, `persistence`, `infra`, `debug`, `core`
- `ui`, `persistence`, `infra`, `debug` depend on `core`
- `core` must not depend on `ui`, `persistence`, `infra`, `debug`, Qt, database drivers, Poppler, OpenCV, Tesseract, `xlnt`, or `libzip`

## Migration Rules

1. Keep capability-oriented ports in `core`.
1. Keep vendor-backed adapters in `infra`.
1. Keep use-case logic in `core/application`.
1. Move technical helpers behind ports instead of adding more nested service layers.
1. Keep request/result/type headers feature-local instead of centralized.
1. Rename public classes before introducing new orchestration code.

## Current Status

Already done:

- `api/` removed
- `services/` removed as a module name
- `infra/` root introduced
- `archive` moved into `infra`
- backend-backed infra adapters extracted behind capability ports
- archive packaging extracted behind `IArchive`
- import path moved toward `core` ports
- QML bootstrap switched to adapter factories
- capability-oriented port wrapper headers added for the new names

Still open:

- remove the remaining legacy aliases once the migration stabilizes
- rename the actual infra folders from backend-oriented implementation trees to capability-based module names if we want the repo tree to mirror the target doc exactly
- rename the actual CMake target prefix from `Infrastructure` to `Infra`
- do a later cleanup slice for the remaining import internals if they still read too much like compatibility code
