# FOSSredder Engineering Standards

This document collects the project-wide engineering rules that are too detailed
for the root contribution guide. The rules apply to feature work, bug fixes,
refactors, documentation work and release preparation.

## Layer Ownership

| Layer | Owns | Must not own |
|---|---|---|
| `core/domain` | Entities, value objects, invariants and reusable policies. | UI state, persistence details or workflow DTO storage. |
| `core/application` | Use-case orchestration, workspace services, imports, exports, analysis and annual workflows. | Qt/QML bindings or concrete third-party libraries. |
| `core/ports` | Public contracts for use cases, workspace access, persistence and infrastructure capabilities. | Concrete implementations. |
| `persistence` | SQLite schema, repositories, registry and workspace state storage. | Domain rules or UI projections. |
| `infra/*` | Concrete adapters for PDF rendering, image processing, OCR, archive and XLSX output. | Product workflows or UI decisions. |
| `ui/src` and `ui/include/ui` | QML-facing state, view models, workflows, adapters, selectors and payload mapping. | Domain invariants, persistence decisions or duplicated business rules. |
| `ui/qml` | Declarative composition, binding, layout, controls and user intent. | Core logic, workflow decisions or data transformation. |

## Refactor Rules

1. Keep changes scoped to the layer and feature being refactored.
2. Preserve observable behavior unless the issue explicitly requests a behavior change.
3. Remove dead wrappers, stale aliases and compatibility paths instead of moving them.
4. Prefer existing project concepts and local patterns over new abstractions.
5. Create shared helpers only when they represent a stable concept, not just to reduce a few repeated lines.
6. Search C++, QML and tests before removing or renaming public API.
7. Keep behavior-critical validation in `core`; UI validation is for immediate feedback and must delegate to core where the rule is business-relevant.

## C++ And Doxygen

- Follow the repository `.clang-format` and `.clang-tidy` configuration.
- Keep includes minimal and ordered in the style already used by the file.
- Prefer clear names over comments that explain unclear names.
- Use `const` and `[[nodiscard]]` where they clarify API intent.
- Avoid catch-all helpers, broad manager classes and compatibility facades.
- Public headers should document files, classes, public functions, public data structures and important enum values with Doxygen.
- Source files should keep comments sparse and factual; add comments only for non-obvious algorithmic or architectural intent.

## UI Source Rules

- View models are narrow QML API surfaces. Public `Q_PROPERTY` and `Q_INVOKABLE` members should map to values QML binds or actions QML triggers.
- View models may hold UI form state such as text fields, selected ids, dirty state, expanded state and button enablement derived from UI state.
- View models must not mutate domain objects, parse business formats, calculate analysis output, finalize imports, export files or inspect aggregates directly.
- Catalog CRUD goes through `WorkspaceCommands`; workspace reads go through `WorkspaceSelectors`; cross-view selection goes through `WorkspaceSelection`.
- Multi-step use cases go through feature workflows and core use-case ports.
- UI adapters translate between Qt/QML-friendly types and core ports. They should mostly map, delegate, bridge cancellation/progress and convert payloads.

## QML Rules

- Keep QML declarative. QML composes UI, binds state and emits user intent.
- Keep non-UI logic in `ui/src` or `core`; do not duplicate backend policies or entity rules in QML.
- Organize feature screens as view families under `ui/qml/FossRedder/Views/<Feature>/`.
- Register new QML files in the owning `qmldir`.
- Prefer shared `Controls`, `Components` and theme values over local one-off patterns.
- Avoid magic colors, spacing, sizes and invisible layout hacks.

## Quality And Traceability

- Add or update tests at the architectural boundary that owns the behavior.
- Update [test matrices](test-matrices.md) when observable behavior or covered test scope changes.
- Update `ci/package/package-layout-contract.json` when a required runtime file, QML module or packaged asset changes.
- Update `ci/localization/localization-contract.json` when supported UI languages or bundled OCR models change.
- Keep test names searchable from matrix IDs when a matrix row maps to code.
- If a verification step was not run, say so explicitly in the pull request.

