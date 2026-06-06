# Infra Testing Matrix

## Purpose

This document defines the concrete test matrix for the `infra` target family.
It focuses on the observable contracts of the infrastructure adapters so each
module remains replaceable, deterministic, and easy to reason about.

The matrix is intentionally family-based: every adapter is tested as a module
capability, not as a one-off implementation story.

## Scope

Included in this matrix:
- `infra/archive/*`
- `infra/image-processing/*`
- `infra/pdf-rendering/*`
- `infra/text-recognition/*`
- `infra/analysis-image-renderer/*`
- `infra/xlsx-writer/*`

Out of scope:
- `core` domain and application rules
- UI behavior
- persistence schema and repository behavior
- end-to-end flows that only appear after multiple infra modules are combined

## Target Test Tree

```text
infra/
  archive/
    tests/
      unit/
        TestZipArchiveAdapter.cpp
  image-processing/
    tests/
      unit/
        TestImageProcessingSupport.h
        TestCropAdapter.cpp
        TestDenoiseAdapter.cpp
        TestDetectAdapter.cpp
        TestMaskAdapter.cpp
  pdf-rendering/
    tests/
      unit/
        TestPdfSupport.h
        TestPopplerCore.cpp
        TestPopplerPdfRendererAdapter.cpp
  text-recognition/
    tests/
      unit/
        TestTesseractCore.cpp
        TestTesseractTextRecognizerAdapter.cpp
  analysis-image-renderer/
    tests/
      unit/
        TestOpenCvAnalysisImageRendererAdapter.cpp
  xlsx-writer/
    tests/
      unit/
        TestXlntTableWriterAdapter.cpp
```

## Testing Principles

- Test each infra module in isolation first.
- Prefer deterministic generated fixtures over fragile external sample files.
- Verify both success and failure paths.
- Check observable outputs, not private helper internals.
- Prefer reusable module-family helpers over special-case setups.
- Keep tests aligned with the module boundary:
  - `archive` tests ZIP packaging and path preservation
  - `image-processing` tests crop, denoise, mask, and detect behavior
  - `pdf-rendering` tests page rendering and metadata extraction
  - `text-recognition` tests OCR extraction and table assembly
  - `analysis-image-renderer` tests analysis overlay rendering and image export
  - `xlsx-writer` tests workbook creation and cell/table serialization

## 1. Archive

`archive` packages export artifacts into ZIP archives.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ARC-001 | Creates a ZIP archive from a nested directory | Integration | Temporary source directory with nested files | Call `IArchive::create(sourceDirectory, archivePath, Zip)` | Archive file is created and contains all files |
| ARC-002 | Preserves relative file paths | Integration | Source directory with nested subfolders | Create archive | Nested paths inside the ZIP match the source-relative layout |
| ARC-003 | Rejects unsupported archive formats | Unit | Archive adapter instance | Call `create(..., format != Zip)` | Returns `false` without writing output |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ARC-B-001 | Adapter class is the only public implementation surface | Unit | Adapter header available | Inspect public API usage in tests | Tests depend on `IArchive` plus the adapter class only |
| ARC-B-002 | ZIP dependency stays isolated inside infra | Contract | Build target for `infra/archive` | Review include dependencies | `core` does not include `zip.h` directly |

## 2. Image Processing

`image-processing` provides OpenCV-backed crop, denoise, mask, and detect
operations for import and analysis workflows.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMG-001 | Crops in-memory images and skips invalid rectangles | Unit | Generated image and a mix of valid and invalid rectangles | Call crop adapter | Valid crops are returned, invalid rectangles are ignored |
| IMG-002 | Writes JPG files when requested | Unit | Generated image and writable output directory | Run crop with JPG output | Output file exists and uses the `.jpg` extension |
| IMG-003 | Denoise supports median mode | Unit | Existing input image file | Request median denoise | Denoised PNG is written to the expected output path |
| IMG-004 | Denoise supports Gaussian mode | Unit | Existing input image file | Request Gaussian denoise | Denoised PNG is written to the expected output path |
| IMG-005 | Denoise supports bilateral mode | Unit | Existing input image file | Request bilateral denoise | Denoised PNG is written to the expected output path |
| IMG-006 | Returns empty result for missing denoise input | Unit | Missing input file path | Call denoise adapter | Result stays empty |
| IMG-007 | Builds mask and whiteout from provided boxes | Unit | Generated image and explicit text boxes | Call mask adapter | Mask bytes, masked bytes, and masked file path are produced |
| IMG-008 | Returns empty result for missing mask input | Unit | Missing source image | Call mask adapter | Result stays empty |
| IMG-009 | Creates line masks for horizontal and vertical structures | Unit | Generated image with line segments | Call `makeLineMask` | Both orientations return non-empty masks |
| IMG-010 | Returns empty results for blank images | Unit | Blank generated image | Call detect adapter | No text blocks and no tables are returned |
| IMG-011 | Detects text blocks on solid dark regions | Unit | Generated image with dark text-like blocks | Call detect adapter | At least one text block is returned |
| IMG-012 | Detects tables on simple grid images | Unit | Generated grid image | Call detect adapter | A table with cells is returned |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMG-B-001 | Public API remains capability-based | Unit | Port headers available | Inspect adapter signatures | API speaks in crop, mask, denoise, detect terms |
| IMG-B-002 | OpenCV dependency stays isolated in infra | Contract | Build target for `infra/image-processing` | Review include dependencies | `core` does not include OpenCV headers directly |
| IMG-B-003 | Internal helpers stay local to the module | Contract | Module test helpers | Inspect include graph | `TestImageProcessingSupport.h` is used only inside infra tests |

## 3. PDF Rendering

`pdf-rendering` provides Poppler-backed PDF rendering and metadata extraction.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PDF-001 | Renders and extracts a single-page PDF | Integration | Generated one-page PDF fixture | Call render and extract entry points | Rendered image bytes and page metadata are produced |
| PDF-002 | Rejects missing PDF input | Unit | Non-existent PDF path | Call render or extract entry point | Operation fails cleanly with an exception |
| PDF-003 | Wraps render and extract contracts through the adapter | Integration | Generated one-page PDF fixture | Call the adapter through `IPdfRenderer` | The adapter forwards render and extract results correctly |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PDF-B-001 | Public API remains capability-based | Unit | Port headers available | Inspect adapter signatures | API speaks in render and metadata terms |
| PDF-B-002 | Poppler dependency stays isolated in infra | Contract | Build target for `infra/pdf-rendering` | Review include dependencies | `core` does not include Poppler headers directly |
| PDF-B-003 | PDF support helper remains test-local | Contract | Module test helpers | Inspect include graph | `TestPdfSupport.h` stays inside the infra test tree |

## 4. Text Recognition

`text-recognition` provides Tesseract-backed OCR and table-aware word assembly.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| OCR-001 | Returns empty output for empty input | Unit | Empty byte array | Call extraction helper | Text and words stay empty |
| OCR-002 | Recognizes simple text and words from a generated image | Integration | Generated text image and bundled tessdata | Call extraction helper | Text and word output is non-empty |
| OCR-003 | Returns empty output for cancelled requests | Unit | Cancel flag already set | Call the adapter | The adapter returns no text, words, or tables |
| OCR-004 | Produces text and table content for a generated image | Integration | Generated text image and one table cell | Call the adapter in table mode | Text, words, and table cell content are populated |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| OCR-B-001 | Public API remains capability-based | Unit | Port headers available | Inspect adapter signatures | API speaks in recognition and word extraction terms |
| OCR-B-002 | Tesseract dependency stays isolated in infra | Contract | Build target for `infra/text-recognition` | Review include dependencies | `core` does not include Tesseract headers directly |
| OCR-B-003 | Tessdata resolution stays inside infra | Contract | Source files available | Review include graph | Tessdata lookup logic remains local to the module |

## 5. Analysis Image Renderer

`analysis-image-renderer` renders analysis overlays and derived image output
without leaking OpenCV details into `core`.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| AIR-001 | Renders a pie analysis image | Integration | Pie analysis result data | Call renderer | Output image file is created and readable |
| AIR-002 | Renders a histogram analysis image | Integration | Histogram analysis result data | Call renderer | Output image file is created and readable |
| AIR-003 | Renders a calculation analysis image | Integration | Calculation analysis result data | Call renderer | Output image file is created and readable |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| AIR-B-001 | Public API remains renderer-focused | Unit | Adapter header available | Inspect adapter signatures | API speaks in render and overlay terms |
| AIR-B-002 | OpenCV and JSON dependencies stay isolated in infra | Contract | Build target for `infra/analysis-image-renderer` | Review include dependencies | `core` does not include OpenCV or nlohmann/json headers directly |
| AIR-B-003 | Adapter naming stays uniform | Contract | Public headers and sources | Inspect class names | Public classes follow the `OpenCvAnalysisImageRendererAdapter` naming rule |

## 6. XLSX Writer

`xlsx-writer` serializes tabular export data into spreadsheet workbooks.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| XLS-001 | Writes a workbook from a simple table | Integration | Table rows and writable path | Call writer | XLSX file is created and contains the expected rows |
| XLS-002 | Preserves sheet title and row/column orientation | Integration | Table rows with known cell layout (for example `A1=Name`, `B1=Value`, `A2=Rent`, `B2=120.50`) | Write workbook | Sheet title and exact cell layout match the input matrix orientation |
| XLS-003 | Serializes formula cells into the OOXML sheet | Unit | Table rows containing a formula cell | Write workbook | The generated sheet XML contains a formula node |
| XLS-004 | Rejects invalid output paths | Unit | Unwritable output path | Write workbook | Operation fails cleanly |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| XLS-B-001 | Public API remains table-writing focused | Unit | Adapter header available | Inspect adapter signatures | API speaks in workbook and table terms |
| XLS-B-002 | xlnt dependency stays isolated in infra | Contract | Build target for `infra/xlsx-writer` | Review include dependencies | `core` does not include xlnt headers directly |
| XLS-B-003 | Adapter naming stays uniform | Contract | Public headers and sources | Inspect class names | Public classes follow the `Xlnt...Adapter` naming rule |

## 7. Direct implementation order

If these tests are implemented in order, the following sequence is the most natural:
1. `archive` tests
2. `image-processing` crop and denoise tests
3. `image-processing` mask and detect tests
4. `pdf-rendering` tests
5. `text-recognition` tests
6. `analysis-image-renderer` tests
7. `xlsx-writer` tests
