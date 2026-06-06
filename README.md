<p align="center">
  <img src="app/assets/icons/fossredder.png" alt="FOSSredder" width="96">
</p>

<h1 align="center">FOSSredder</h1>

<p align="center">
  <a href="https://github.com/Wiiii90/fossredder/actions/workflows/quality.yml?query=branch%3Amaster"><img alt="Stable CI" src="https://img.shields.io/github/actions/workflow/status/Wiiii90/fossredder/quality.yml?branch=master&label=stable%20ci&style=flat-square"></a>
  <a href="https://codecov.io/gh/Wiiii90/fossredder/branch/master"><img alt="Stable coverage" src="https://img.shields.io/codecov/c/github/Wiiii90/fossredder/master?token=LGALNE53Z6&label=stable%20coverage&style=flat-square"></a>
  <a href="https://github.com/Wiiii90/fossredder/actions/workflows/quality.yml?query=branch%3Adevelop"><img alt="Nightly CI" src="https://img.shields.io/github/actions/workflow/status/Wiiii90/fossredder/quality.yml?branch=develop&label=nightly%20ci&style=flat-square"></a>
  <a href="https://codecov.io/gh/Wiiii90/fossredder/branch/develop"><img alt="Nightly coverage" src="https://img.shields.io/codecov/c/github/Wiiii90/fossredder/develop?token=LGALNE53Z6&label=nightly%20coverage&style=flat-square"></a>
</p>

**FOSSredder** is a deliberately overengineered Windows desktop application built for a concrete, real-world use case: extracting structured data from PDF bank statements issued by Commerzbank, a major German banking institution, in order to automate the annual allocation of recoverable costs to tenants.

The application replaces a manual, Excel-based workflow with a semi-automated pipeline. It extracts transaction data from PDF statements using OCR and heuristic parsing, followed by a custom matching engine that pre-fills entries by linking them to specific actors, properties, and contracts. A dedicated review interface allows for human-in-the-loop validation, streamlining the process of identifying recoverable costs before the final export back into a structured Excel format.

While the core problem could be addressed with simple scripts or well-known LLMs, this project also serves as an engineering exercise to explore modular system design, extensibility, and modern development workflows:
* **Modular System Design:** Built with decoupled CMake targets (core, persistence, ui, services) to ensure a clear separation of concerns and high maintainability.
* **Extensible Architecture:** Designed to support additional document formats through a strategy-based approach that leverages internal service adapters for OCR and parsing.
* **Modern Development:** Built using an iterative, AI-assisted workflow with a strong emphasis on Clean Architecture principles.
* **Privacy by Design:** Since financial data is sensitive, the system runs strictly locally. No cloud, no external APIs.

## Technology Stack

| | |
| --- | --- |
| Application | C++20, Qt 6, QML / Qt Quick |
| Architecture | Modular CMake targets for `core`, `persistence`, `ui`, `infra`, and `app` |
| Data & Persistence | SQLite, `nlohmann-json` |
| Document Processing | Poppler, OpenCV, Tesseract |
| Testing & Quality | GoogleTest, clang-tidy, LLVM coverage, Codecov |
| Build & Packaging | CMake, vcpkg, Inno Setup, GitHub Actions |
| Documentation | Doxygen, GitHub Pages |
| Platform | Windows 10+ |

## Project Status
This application is in **Active Alpha** and has reached its first named baseline milestone. The current codebase is installable, launchable, and functional for the core Commerzbank statement workflow, while still being actively hardened and refined.
* **Core Functionality:** Commerzbank PDF import, XLSX and CSV export, persistence, and the refactored UI/core integration are implemented and usable.
* **Current Focus:** Stabilizing the `v0.5.0` baseline, improving the matcher and workflow configuration, and tightening the calculation and domain model.
* **Near-term Work:** Packaging and installer hardening, GitHub Actions and Doxygen automation, contract and golden tests, and release-readiness polish.

## Getting started

### Prerequisites

The project uses CMake (minimum 4.0.1) with the `Visual Studio 18 2026` generator (x64) and `vcpkg` in manifest mode for dependency management.

Install vcpkg (install to a short path, e.g. `C:\vcpkg`):

```powershell
git clone https://github.com/microsoft/vcpkg.git <path-to-vcpkg>
```

Bootstrap vcpkg (prepare vcpkg for use with this repository).

```powershell
<path-to-vcpkg>\bootstrap-vcpkg.bat
```

Set vcpkg root (set `VCPKG_ROOT` to the vcpkg path so CMake can find the toolchain).

```powershell
[Environment]::SetEnvironmentVariable('VCPKG_ROOT', '<path-to-vcpkg>', 'User')
```

The project uses `CMakePresets.json` for presets. List available presets:

```powershell
cmake --list-presets
```

Choose a configure preset and run `cmake --preset <name>` to configure. Examples are given in the upcoming chapters.

**Note:** CMake should pick up the vcpkg toolchain automatically when using these presets because the project calls `fossredder_configure_vcpkg()` in `CMakeLists.txt`. If your environment does not, set the `VCPKG_ROOT` environment variable and re-run the configure preset.

### Building

The default workflow configures app and provides debug-app and release-app build variants.

Configure (app preset):

```powershell
cmake --preset app
```

Build debug app (includes debug symbols and has assertions enabled)

```powershell
cmake --build --preset debug-app
```

Executable: `./.build/app/bin/Debug/fossredder.exe`

Build release app (optimized for distribution and packaging):

```powershell
cmake --build --preset release-app
```

Executable: `./.build/app/bin/Release/fossredder.exe`

### Testing

The test suite uses GoogleTest and includes unit, interaction, UI/QML, persistence and debug-module tests.

Configure (tests preset):

```powershell
cmake --preset tests
```

Build release tests:

```powershell
cmake --build --preset release-tests
```

Run tests (in console only):

```powershell
ctest --preset release-tests --output-on-failure
```

### Packaging

The project uses Inno Setup for Windows installer packaging through the `release-package` preset.
Validated develop installers are published as the mutable `develop-nightly`
pre-release in GitHub Releases. Stable installers should be promoted from
`master` as versioned releases.

Build release package (creates the Windows installer via Inno Setup):

```powershell
cmake --build --preset release-package
```

Installer: `./.build/app/dist/FOSSredder-Setup-<version>-win-x64.exe`

### Quality Workflow

The project uses clang-tidy for static analysis, LLVM coverage for coverage reports, and Codecov for publishing coverage results from the GitHub Actions workflow in `.github/workflows/quality.yml`. The same tools can also be run locally.

Notes: Coverage requires Clang/`clang-cl` and `clang-tidy` runs only if installed and the `tidy` preset is used.

#### CI Workflow

Configure CI (ci preset):

```powershell
cmake --preset ci
```

Build release ci:

```powershell
cmake --build --preset release-ci
```

Run CI tests:

```powershell
ctest --preset release-ci --output-on-failure
```

#### Clang-Tidy

Configure tidy:

```powershell
cmake --preset tidy
```

Build release tidy (runs static analysis during build):

```powershell
cmake --build --preset release-tidy
```

#### Coverage

Configure coverage:

```powershell
cmake --preset coverage
```

Build release coverage (builds the instrumented test binaries):

```powershell
cmake --build --preset release-coverage
```

Run coverage tests (collects coverage data):

```powershell
ctest --preset release-coverage --output-on-failure
```

Coverage output: `./coverage/` and `./coverage/coverage.lcov`.

## Docs

- `docs/` contains design artifacts and requirements. The implementation (code) should be considered the authoritative source when documentation and code disagree.
- All source headers are documented using Doxygen-style comments (`@brief`, `@param`, etc.). You can generate the full HTML reference by running `doxygen Doxyfile` in the project root.
- GitHub Actions publishes the generated Doxygen HTML as the `fossredder-doxygen-html` artifact from the docs workflow.

## License

This project is licensed under the MIT License. See `LICENSE` for details.
