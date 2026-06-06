# UI Source Rules

These rules describe the `ui/src` and `ui/include/ui` refactor standard used for the C++ frontend boundary cleanup.

For version 0.5, the ViewModel rules below are the production target: each
ViewModel must be a narrow QML API surface, and any remaining implementation
detail must be local UI form or presentation state.

## Architecture

- `ui/src` is the Qt-facing presentation boundary. It exposes QML-ready state, receives user intent, and routes that intent to workspace, workflows, or settings.
- Do not duplicate domain rules in `ui/src`. Entity invariants, validation policies, calculation rules, draft mutation rules, import parsing, export semantics, and analysis aggregation belong in `core/domain` or `core/application`.
- Keep one source of truth. Do not introduce local shadow entities, copied catalogs, local run/log stores, compatibility caches, or fallback paths when workspace or workflow state already owns the data.
- Use typed core ports and DTOs at boundaries. Do not pass opaque JSON blobs through `ui/src` unless the core contract explicitly defines the value as serialized payload.
- Preserve behavior during refactors unless the change is explicitly requested. Naming cleanup, file moves, and consolidation must not silently change selection, CRUD, import/export, analysis, annual, settings, or navigation behavior.

## ViewModels

- ViewModels are the QML API surface for a view or view family. Public `Q_PROPERTY` and `Q_INVOKABLE` members should correspond to values QML binds or user actions QML triggers.
- ViewModels may hold UI form state: current text field contents, selected row ids, selected chip indices, dirty-state snapshots, expanded/collapsed presentation state, and button enablement derived from UI state.
- ViewModels must not mutate domain objects, parse business formats, calculate analysis results, finalize imports, export files, persist data, or inspect core aggregates directly.
- CRUD commands from ViewModels go through `WorkspaceCommands`. Workspace reads go through `WorkspaceSelectors` and `WorkspaceSelection`. Multi-step use cases go through the relevant workflow. Settings commands go through the shell/settings object or workspace path established for settings.
- ViewModels may do immediate UX checks for disabled buttons, but authoritative validation must come from core policies through workspace/workflow ports.
- Private helpers should support the public QML API. Remove wrappers that only rename a single workspace or workflow call without adding real ViewModel responsibility.
- Keep feature ViewModels self-contained for now. Do not create shared ViewModel helper files or make ViewModels reference each other just to remove local repetition.
- Prefer feature-level names over generic legacy names. For example, `actorFormStateFromRow` is clearer than `basicFormState`, and `reloadFormState` is clearer than `reloadLinkedFormState`.

## Workspace

- `ui/src/workspace` is the Qt-facing workspace module. Keep it physically grouped and flat, but split responsibility into `WorkspaceStore`, `WorkspaceCommands`, `WorkspaceSelection`, `WorkspaceSelectors`, and `WorkspacePayloads`.
- `WorkspaceStore` owns the current `core::ports::workspace::WorkspaceSnapshot`, bound workspace ports needed for refresh, and the UI data revision. It is the only UI-side source of truth for loaded workspace data.
- `WorkspaceCommands` is the only UI-side mutation path for workspace state. It maps UI input to core workspace command DTOs, delegates authoritative validation and mutation to `IWorkspaceWriter`, and maps validation results to QML payloads.
- `WorkspaceSelection` owns cross-view selected ids and keeps them valid against the current store data. It does not submit commands, build feature form state, persist files, or call workflows.
- `WorkspaceSelectors` are pure read-only projections from `WorkspaceStore` and `WorkspaceSelection`. They build QML-facing rows, dropdowns, lookup maps, selected-row payloads, and filtered row lists. They must not mutate state, call writer ports, or perform persistence.
- `WorkspacePayloads` contains workspace-specific `QVariantMap`/`QVariantList` construction helpers and validation payload mapping. Generic Qt conversions remain in `ui/presentation/PayloadMapper`.
- Workspace is the only source-side entry point for catalog CRUD from ViewModels: actors, properties, contracts, statements, transactions, analyses, annuals, logs, and drafts are routed through workspace commands when they are workspace state.
- Workspace validation calls must delegate to core workspace command validation. UI-side validation maps core issues to QML payloads; it does not invent a second rule set.
- Avoid full-snapshot scans in ViewModels for specific entity lookups. Put read-model derivation in `WorkspaceSelectors`, and add focused core reader queries deliberately and consistently when the UI truly needs a core-side query.
- Qt `QAbstractItemModel` workspace adapters are not part of the v0.5 target architecture. Do not create or keep a ListModel for a workspace entity unless QML has a concrete `QAbstractItemModel`, proxy/filter, or role-binding need that selectors cannot satisfy.
- Avoid parallel read models. A workspace entity should not have both selector payloads and a Qt ListModel path unless the concrete consumer of each path is documented.
- Do not introduce `WorkspaceFacade`, compatibility facades, manager-style wrappers, or forwarding aggregates. ViewModels depend on the specific workspace role objects they need.

## Workflows

- Workflows orchestrate feature use cases that are larger than one CRUD command: import, export, analysis computation, and annual computation.
- Workflows own lifecycle concerns: start, pause, resume, cancel, progress, completion, failure, and user-in-the-loop continuation.
- Workflows may maintain transient use-case state needed by QML, such as current import progress or export status. Historical logs and persisted workspace state should be written through workspace where applicable.
- Workflows must not parse files, calculate analysis data, mutate drafts according to business rules, or assemble export contents themselves when a core runner/service owns that use case.
- A small workflow is still a valid workflow when the feature shape requires consistency with the other use cases. Do not remove a workflow only because its current implementation is short.

## Adapters

- UI adapters translate between Qt/QML-friendly types and core use-case ports. They should be mostly mapping, delegation, cancellation/progress bridging, and payload conversion.
- Adapters call core runner ports. They do not call QML, do not own ViewModel state, and do not mutate workspace catalogs directly unless the core port contract explicitly requires it.
- Keep adapters dumb. If adapter code starts deriving actors, properties, contracts, amounts, analysis tables, import drafts, export items, or annual summaries, move that logic to core application/domain.
- Prefer adapting existing core DTOs over creating parallel UI DTO families. If the adapter needs a shape the port does not expose, improve the port deliberately.

## Core Boundary

- Core owns business truth. `core/domain` owns entities, value objects, and policies. `core/application` owns use-case services, command services, workflow support, and projection logic that is not Qt-specific.
- `core/ports/usecases/*` define feature runner contracts. `core/ports/workspace` defines workspace reader/writer contracts. `core/ports/infra/*` define external technical capabilities.
- UI may depend on core ports and DTOs. UI must not depend on core application internals when a port exists or should exist.
- If a feature needs frontend control such as pause, resume, cancel, progress, or validation, expose it through the use-case/workspace port instead of creating a UI-only simulation.

## Naming And Files

- Keep `ui/src/viewmodels` flat and feature-based. A feature ViewModel should not be split into artificial `Choices`, `Form`, `Internals`, or `ContractFields` files.
- Prefer one ViewModel per QML feature surface unless a child ViewModel is a real QML object with its own public API.
- Keep workflow folders flat by feature when possible: `ImportWorkflow`, `ExportWorkflow`, `AnalysisWorkflow`, and `AnnualWorkflow`.
- Keep file names aligned with the public concept they expose. Avoid suffixes such as `State`, `Session`, `Facade`, `Manager`, or `Support` unless the file truly owns that role.
- Use `Store`, `Commands`, `Selection`, `Selectors`, and `Payloads` only for those exact workspace roles.
- Avoid `Projector` for workspace UI read-model names. Prefer `Selectors` for read-only derivation and `Payloads` for QML payload construction.
- Remove dead files and wrappers during refactors. Do not move stale code into private headers or new folders to hide it from review.
- Consolidation target for ViewModels is behavioral clarity, not lowest line count. Local duplication is acceptable when the alternative is a premature shared abstraction.

## Observability

- Trace at boundaries: ViewModel user intent, workspace command acceptance/rejection, workflow lifecycle events, adapter runner calls, and storage/open/save operations.
- Do not add debug logging to entity setters, low-level value-object normalization, or hot inner loops unless diagnosing a specific defect.
- Validation failures should produce structured field/code/message data that QML can render consistently.
- Debug traces must help identify the boundary and intent. Include ids, names, operation names, counts, status, and first validation issue where useful.

## Refactor Checklist

1. Identify the QML entry points that call the ViewModel.
2. List public ViewModel functions and mark which are directly used by QML.
3. Remove private wrappers that only forward to workspace or workflow.
4. Move business logic from ViewModels, workflows, and adapters into core application/domain.
5. Keep UI-only form state in ViewModels and QML-only rendering in QML.
6. Route CRUD through `WorkspaceCommands`, workspace reads through `WorkspaceSelectors`, selection through `WorkspaceSelection`, and multi-step use cases through workflows.
7. Route workflow runner calls through adapters and core use-case ports.
8. Delete dead shadow state, compatibility fallbacks, duplicate caches, and obsolete files.
9. Update tests and design matrices when public ViewModel API or layer responsibilities change.
