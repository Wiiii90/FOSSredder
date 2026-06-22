# Design Appendix

## Glossary

| Term | Meaning |
|---|---|
| <a id="glossary-domain-driven-design"></a>Domain-Driven Design (DDD) | Design approach that models software around business concepts, language and invariants. In FOSSredder this means core owns entities, values, policies and use cases rather than Qt or database details. |
| <a id="glossary-clean-architecture"></a>Clean Architecture | Dependency model where domain and application rules sit at the center, while UI, persistence, frameworks and external libraries stay outside and depend inward through ports. |
| <a id="glossary-composition-root"></a>Composition root | Startup boundary that is allowed to know concrete implementations and wire them together. In FOSSredder this is the `app` target. |
| <a id="glossary-mvvm"></a>Model-View-ViewModel (MVVM) | UI architecture where QML views bind to `QObject` view models, while model data comes from core snapshots, commands and use-case ports. |
| <a id="glossary-dto"></a>Data Transfer Object (DTO) | Plain request, result or snapshot structure used to cross architectural boundaries without exposing mutable domain objects. |
| <a id="glossary-domain-language"></a>Domain language | The project vocabulary used by the product and the code, for example actor, property, contract, statement, transaction, analysis and annual. |
| <a id="glossary-domain-entity"></a>Domain entity | A domain object with identity and lifecycle, such as `Actor`, `Contract`, `Transaction` or `Annual`. |
| <a id="glossary-value-object"></a>Value object | Immutable or normalization-focused domain type identified by its value rather than an id, such as `EntityName`, `MoneyAmount` or `Year`. |
| <a id="glossary-policy"></a>Policy | A named domain rule object for behavior that should not be hidden in UI, persistence or ad-hoc helpers. |
| <a id="glossary-use-case"></a>Use case | Application-level operation that coordinates domain objects and ports to perform user-visible work. |
| <a id="glossary-port"></a>Port | Core-defined interface or DTO boundary consumed by UI, persistence or infrastructure. Ports keep dependency direction pointing inward. |
| <a id="glossary-adapter"></a>Adapter | Concrete implementation that translates between a port and an external library, service or UI workflow. |
| <a id="glossary-repository"></a>Repository | Persistence interface or implementation responsible for storing and loading one family of domain/application records. |
| <a id="glossary-workspace"></a>Workspace | User-managed `.fossredder` SQLite file containing catalog entities, workflow state and logs. |
| <a id="glossary-registry"></a>Registry | Small local `registry.db` used to remember the latest workspace path. |
| <a id="glossary-catalog"></a>Catalog | Domain aggregate containing actors, properties, contracts, statements, transactions, analyses and annuals. |
| <a id="glossary-session-state"></a>Workspace session state | In-memory application state for the active workspace, including catalog data and workflow records. |
| <a id="glossary-snapshot"></a>Snapshot | Read model projected from workspace state for UI or use-case consumption. |
| <a id="glossary-draft"></a>Draft | Import-time statement or transaction state that can be reviewed before finalization. |
| <a id="glossary-import-log"></a>Import log | Persisted workflow record describing an import run, its status and related draft/final statement. |
| <a id="glossary-export-log"></a>Export log | Persisted workflow record describing an export run, its status and produced output metadata. |
| <a id="glossary-view-model"></a>View model | QML-facing `QObject` that owns presentation state for one feature area. |
| <a id="glossary-workflow"></a>Workflow | Coordinator for multi-step or asynchronous behavior, for example import, analysis, annual or export execution. |
| <a id="glossary-qml-module"></a>QML module | Importable QML package described by `qmldir` files and loaded by the Qt QML engine. |
| <a id="glossary-cmake-preset"></a>CMake preset | Named CMake configure/build/test entry in `CMakePresets.json`. Presets are the supported way to describe repeatable local and CI builds. |
| <a id="glossary-test-matrix"></a>Test matrix | Table that maps behavior families to expected tests and implementation locations. |
| <a id="glossary-quality-gate"></a>Quality gate | Required validation step that must pass before a branch, release or installer artifact is considered acceptable. |
| <a id="glossary-ci-pipeline"></a>CI pipeline | GitHub Actions workflow that validates build, tests, static analysis, coverage, documentation, Pages and develop-nightly packaging. |
| <a id="glossary-static-analysis"></a>Static analysis | Source/build analysis that checks code without relying only on runtime behavior. The current configured tool is clang-tidy. |
| <a id="glossary-coverage"></a>Coverage | Measurement of which code lines or regions are exercised by tests. FOSSredder generates LLVM/LCOV coverage and uploads Codecov input. |
| <a id="glossary-doxygen"></a>Doxygen | Documentation generator used to produce HTML documentation from source comments and configured inputs. |
| <a id="glossary-pages"></a>GitHub Pages | Hosted GitHub site used here for generated documentation and coverage reports. |
| <a id="glossary-artifact"></a>Artifact | File or directory produced by CI, for example coverage HTML, Doxygen HTML, package logs or installer output. |
| <a id="glossary-installer"></a>Installer | Windows setup executable produced by Inno Setup and attached to nightly or stable releases. |
| <a id="glossary-runtime-layout"></a>Runtime layout | Installed file structure required for the application to start correctly, including executable, Qt plugins, QML modules, translations and tessdata. |
| <a id="glossary-tessdata"></a>Tessdata | Tesseract OCR language and orientation model files bundled with the application. |
| <a id="glossary-release-channel"></a>Release channel | Distribution path with its own trigger and audience, for example develop nightly or stable tagged release. |
| <a id="glossary-develop-nightly"></a>Develop nightly | Mutable GitHub pre-release built from validated `develop` pipeline runs. |
| <a id="glossary-stable-release"></a>Stable release | GitHub Release built from an immutable `v*` tag. |
| <a id="glossary-code-signing"></a>Code signing | Cryptographic signing of installer/executable artifacts so Windows can identify the publisher. |
| <a id="glossary-threat-model"></a>Threat model | Structured view of relevant risks, trust boundaries and mitigations. |
| <a id="glossary-at-rest-encryption"></a>At-rest encryption | Encryption of stored files such as the workspace database while they are not actively in use. |
| <a id="glossary-telemetry"></a>Telemetry | Automatic runtime collection or upload of usage, diagnostic or environment data. |
| <a id="glossary-runner"></a>Runner | Machine that executes GitHub Actions jobs. FOSSredder uses a self-hosted Windows runner for Windows build jobs. |
| <a id="glossary-vcpkg-manifest"></a>vcpkg manifest | `vcpkg.json` dependency declaration used by CMake/vcpkg manifest mode. |

## External Dependencies

Dependencies are managed through `vcpkg.json`. The manifest is the authoritative
source for third-party dependency names; product versioning is owned by
`project(FossRedder VERSION ...)` in the root `CMakeLists.txt`.

| Role | Dependencies |
|---|---|
| UI | `qtbase`, `qtdeclarative`, `qtquickcontrols2`, `qttools`, `qtsvg`, `qtimageformats`, `qtshadertools` |
| PDF rendering | `poppler` |
| Document image processing | `document-image-processing` |
| OCR | `tesseract`, `leptonica`, runtime `tessdata` files |
| Data and export | `nlohmann-json`, `xlnt`, `protobuf`, `libzip` |
| Logging and support | `spdlog`, `icu`, `pkgconf` |
| Tests | `gtest` |

## Important Runtime Files

| File or directory | Meaning |
|---|---|
| `core/include/core/constants/runtime.h` | Canonical runtime filenames such as workspace and registry names. |
| `app/i18n` | Translation sources and generated catalogs. |
| `infra/text-recognition/res/tessdata` | OCR models bundled into the installed runtime. |
| `ui/qml/FossRedder` | Application QML module tree. |
| `installer/inno` | Inno Setup definition and include files. |
| `ci/package/package-layout-contract.json` | Installer/runtime layout contract. |
| `ci/localization/localization-contract.json` | Supported language and OCR model contract. |

## Reference Index

| Concept | Representative files |
|---|---|
| Application entry and composition | `app/src/main.cpp`, `app/src/main_qml.cpp` |
| CMake presets | `CMakePresets.json` |
| CMake project version | `CMakeLists.txt` |
| Core domain model | `core/include/core/domain`, `core/src/domain` |
| Core application services | `core/include/core/application`, `core/src/application` |
| Core ports | `core/include/core/ports` |
| Workspace session and facade | `core/include/core/application/workspace`, `core/src/application/workspace` |
| Storage orchestration | `core/include/core/application/storage/StorageManager.h`, `core/src/application/storage/StorageManager.cpp` |
| Schema and migrations | `persistence/src/SqliteSchema.cpp`, `persistence/include/persistence/SqliteSchema.h` |
| Workspace state store | `persistence/src/WorkspaceStateStore.cpp`, `persistence/include/persistence/WorkspaceStateStore.h` |
| Registry | `persistence/src/SqliteRegistry.cpp`, `core/include/core/ports/infra/storage/IRegistry.h` |
| Repository implementations | `persistence/src/repositories`, `persistence/include/persistence/repositories` |
| Import application | `core/src/application/import`, `core/include/core/application/import` |
| PDF adapter | `infra/pdf-rendering` |
| document-image-processing adapter | `infra/document-image-processing` |
| OCR adapter and tessdata | `infra/text-recognition` |
| XLSX writer adapter | `infra/xlsx-writer` |
| Archive adapter | `infra/archive` |
| Analysis image renderer | `infra/analysis-rendering` |
| UI shell | `ui/include/ui/shell`, `ui/src/shell`, `ui/qml/FossRedder/Components` |
| UI workspace roles | `ui/include/ui/workspace`, `ui/src/workspace` |
| UI view models and workflows | `ui/include/ui/viewmodels`, `ui/src/viewmodels`, `ui/include/ui/workflows`, `ui/src/workflows` |
| QML views and controls | `ui/qml/FossRedder/Views`, `ui/qml/FossRedder/Controls` |
| Tests | `core/tests`, `persistence/tests`, `infra/*/tests`, `ui/tests` |
| Pipeline workflow | `.github/workflows/pipeline.yml` |
| Release workflow | `.github/workflows/release.yml` |
| Packaging scripts | `ci/package`, `installer` |
| Coverage and Pages | `ci/coverage`, `ci/pages`, `Doxyfile` |

## Quality Traceability

The full row-level test matrices live in [docs/appendix/test-matrices.md](test-matrices.md). This design document keeps the QA strategy and gate model in Chapter 7; the dedicated matrix document provides traceability across core, infrastructure, UI, QML and deployment/runtime QA.
