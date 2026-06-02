# Core Domain Refactor 2

This is the concrete target plan for the next `core` refactor wave.

The goal is strict:

- domain objects are not read/write data bags
- `WorkspaceState` disappears as a domain hybrid
- UI consumes only `core/ports`
- ports expose snapshots and commands, not domain entities
- workspace workflow state stays outside the domain catalog

## Final Target Tree

Only changed or newly introduced areas are listed here. Existing modules that
are not named in this plan stay where they are.

```text
core/
  include/core/
    domain/
      catalog/
        WorkspaceCatalog.h
      entities/
        Actor.h
        Property.h
        Contract.h
        Statement.h
        Transaction.h
        Analysis.h
        Annual.h
      values/
        Alias.h
        EntityName.h
        ContractType.h
        AnalysisType.h
        ExportFormat.h
        FilterSpec.h
        BookingDate.h
        MoneyAmount.h
        Year.h
      policies/
        AliasPolicy.h
        TransactionPolicy.h
        AnalysisPolicy.h
        AnnualPolicy.h
        DraftMatchingPolicy.h
    application/
      workspace/
        WorkspaceFacade.h
        WorkspaceSession.h
        WorkspaceSessionState.h
        WorkspaceStateManager.h
        WorkspaceCommandService.h
        WorkspaceWorkflowService.h
        WorkspaceQueryService.h
        WorkspaceSnapshotProjector.h
      import/
        draft/
          StatementDraft.h
          TransactionDraft.h
          DraftFinalizer.h
          DraftMatcher.h
    ports/
      workspace/
        IWorkspaceReader.h
        IWorkspaceWriter.h
        WorkspaceSnapshot.h
        WorkspaceCommands.h
```

The `src/` tree mirrors this structure.

## Workspace Target

`WorkspaceState` is removed from `core/domain`.

The replacement is split by responsibility:

- `WorkspaceCatalog` is the domain aggregate for catalog entities only.
- `WorkspaceSessionState` is the internal mutable application state.
- `WorkspaceSnapshot` is the immutable read boundary returned by ports.
- `WorkspaceCommands` contains write-side boundary request types.

There is no public `WorkspaceDocument` target type.

### Workspace Session State

`WorkspaceSessionState` is internal to `core/application/workspace`.

It contains:

- `WorkspaceCatalog catalog`
- statement drafts
- transaction drafts
- import logs
- export logs

It is used by `WorkspaceSession`, `WorkspaceStateManager`, and workspace
services. It is not returned to UI.

### Workspace Snapshot

`WorkspaceSnapshot.h` is the read model for ports.

It should contain plain snapshot rows:

- `ActorSnapshot`
- `PropertySnapshot`
- `ContractSnapshot`
- `StatementSnapshot`
- `TransactionSnapshot`
- `AnalysisSnapshot`
- `AnnualSnapshot`
- `StatementDraftSnapshot`
- `ImportLogSnapshot`
- `ExportLogSnapshot`

`IWorkspaceReader` returns `WorkspaceSnapshot`, not `WorkspaceSessionState` and
not domain entities.

`WorkspaceSnapshot` also carries workspace metadata such as the current path and
whether a file is open.

### Workspace Commands

`WorkspaceCommands.h` contains boundary command structs:

- `ActorCommand`
- `PropertyCommand`
- `ContractCommand`
- `StatementCommand`
- `TransactionCommand`
- `AnalysisCommand`
- `AnnualCommand`
- `StatementDraftCommand`
- `FinalizeStatementDraftCommand`
- `ImportLogsCommand`
- `ExportLogsCommand`

`IWorkspaceWriter` takes these commands instead of long primitive parameter
lists.

## Workspace File Mapping

| Current | Target |
| --- | --- |
| `core/include/core/domain/WorkspaceState.h` | `core/include/core/domain/catalog/WorkspaceCatalog.h` |
| `core/src/domain/WorkspaceState.cpp` | `core/src/domain/catalog/WorkspaceCatalog.cpp` |
| `core/include/core/application/workspace/Workspace.h` | `core/include/core/application/workspace/WorkspaceSessionState.h` plus `core/include/core/ports/workspace/WorkspaceSnapshot.h` |
| `core/include/core/application/workspace/IWorkspaceReader.h` | `core/include/core/ports/workspace/IWorkspaceReader.h` |
| `core/include/core/application/workspace/IWorkspaceWriter.h` | `core/include/core/ports/workspace/IWorkspaceWriter.h` |
| `core/include/core/application/workspace/WorkspaceFacade.h` | keep name and location |
| `core/include/core/application/workspace/WorkspaceSession.h` | keep name and location |
| `core/include/core/application/workspace/WorkspaceStateManager.h` | keep name and location |
| `core/include/core/application/workspace/WorkspaceQueryService.h` | keep name and location |
| `core/include/core/application/workspace/WorkspaceMutationService.h` | rename to `WorkspaceCommandService.h` |
| `core/include/core/application/workspace/WorkspaceCatalogService.h` | fold into `WorkspaceCatalog` and `WorkspaceCommandService` |
| `core/include/core/application/workspace/WorkspaceDraftMutationService.h` | fold into `WorkspaceWorkflowService.h` |
| `core/include/core/application/workspace/WorkspaceLogMutationService.h` | fold into `WorkspaceWorkflowService.h` |
| `core/src/application/workspace/CatalogDraftAppliers.h` | fold into `WorkspaceCatalog.cpp` or `WorkspaceCommandService.cpp` |
| `core/src/application/workspace/CatalogMutationHelpers.h` | fold into `WorkspaceCatalog.cpp` or `WorkspaceCommandService.cpp` |

## Workspace Services

The final workspace module should stay flat. There are not enough files to
justify subfolders such as `facade/`, `session/`, `query/`, `catalog/`,
`draft/`, and `log/`.

Keep these public application services:

- `WorkspaceFacade`: public implementation of reader/writer ports.
- `WorkspaceSession`: owns current path, storage manager, callbacks, and mutable session state.
- `WorkspaceStateManager`: loads/saves `WorkspaceSessionState` through repositories.
- `WorkspaceCommandService`: handles catalog write commands and commits.
- `WorkspaceWorkflowService`: handles drafts, import logs, and export logs.
- `WorkspaceQueryService`: answers read queries through snapshots.
- `WorkspaceSnapshotProjector`: maps `WorkspaceSessionState` to `WorkspaceSnapshot`.

Remove these public services:

- `WorkspaceCatalogService`
- `WorkspaceDraftMutationService`
- `WorkspaceLogMutationService`

## Port Shape

`IWorkspaceReader` should expose:

- `WorkspaceSnapshot workspaceSnapshot() const`
- `std::optional<StatementDraftSnapshot> statementDraftSnapshot(const std::string& draftId = {}) const`

`IWorkspaceWriter` should expose:

- workspace lifecycle methods
- catalog command methods using `WorkspaceCommands`
- draft workflow methods
- log update methods if logs remain writable from outside
- callback registration using `WorkspaceSnapshot`

No port method should return `WorkspaceCatalog`, `WorkspaceSessionState`, or
domain entities. No port method should expose `core/application/import/draft`
types directly.

## Domain Entity Rule

Domain entities must stop acting as generic read models.

Remove as default API:

- mutable reference getters
- broad raw state getters used by UI
- mirror getter/setter pairs with no behavior
- workflow-only state

Keep:

- behavior methods
- explicit mutation methods
- invariants
- minimal query methods needed by domain policies

## Alias Consolidation

The alias value family is consolidated.

Target tree:

```text
core/
  include/core/domain/values/
    Alias.h
  src/domain/values/
    Alias.cpp
```

Mapping:

| Current | Target |
| --- | --- |
| `core/include/core/domain/values/Alias.h` | canonical alias value object |
| `core/src/domain/values/Alias.cpp` | canonical alias implementation |
| `core/include/core/domain/values/AliasValue.h` | fold into `Alias.h` |
| `core/src/domain/values/AliasValue.cpp` | remove |
| `core/include/core/domain/values/AliasUsage.h` | remove unless it has real invariants |
| `core/src/domain/values/AliasUsage.cpp` | remove unless `AliasUsage` survives |

Decision:

- `AliasValue` should disappear if it only wraps normalized text.
- `AliasUsage` should disappear from `values` if it is only usage bookkeeping.
- usage/history data belongs in the owning entity or the snapshot if it has no
  independent domain behavior.

## Analysis Tightening

`Analysis` should stop exposing JSON blobs as its main model.

Mapping:

| Current | Target |
| --- | --- |
| `Analysis.configJson` | typed analysis config |
| `Analysis.filterSpec` | typed filter config |
| `Analysis.exportStateJson` | typed output snapshot |
| `Analysis.snapshotTransactionsJson` | typed output snapshot |
| `Analysis.adjustments` | typed adjustment model |

## Drafts

Drafts remain in `core/application/import/draft`.

They are workflow models, not domain entities. Finalization may call domain
rules and mutate the catalog aggregate, but application code must not duplicate
domain invariants.

## UI Alignment

`UI_ARCHITECTURE_TARGET.md` is still directionally good.

The strict core-aligned rule is:

- UI reads `core/ports/workspace/IWorkspaceReader`
- UI writes through `core/ports/workspace/IWorkspaceWriter`
- UI receives `WorkspaceSnapshot`
- UI maps snapshots to Qt models in `ui/adapters` and `ui/viewmodels`
- UI does not include `core/domain/*` or `core/application/*`

Relevant UI mapping stays:

| Current | Target |
| --- | --- |
| `ui/src/state/StateFacadeProjection.cpp` | `ui/src/adapters/WorkspaceRowProjector.cpp` |
| `ui/src/state/WorkspaceClone.*` | `ui/src/application/snapshots/WorkspaceSnapshotFactory.*` or remove if core snapshot is sufficient |
| `ui/src/import/ImportJobBridge.*` | `ui/src/application/jobs/ImportStatementJobClient.*` |
| `ui/src/import/ImportRunStore.*` | `ui/src/application/runs/RunArtifactStore.*` |
| `ui/src/export/ExportRunner.*` | `ui/src/application/workflows/export/ExportWorkflow.*` |
| `ui/src/import/ImportState.*` | `ui/src/application/workflows/import/ImportWorkflowState.*` |

## Stable Areas

Do not move files outside the named mappings unless a compile dependency forces
it.

Stable by default:

- `core/include/core/constants/*`
- `core/src/application/analysis/*`
- `core/src/application/export/*`
- `core/src/application/import/*` except the draft touchpoints above
- `ui/qml/*`
- `ui/shared/*`
- `ui/platform/*`

## Definition Of Done

This refactor wave is done when:

- `WorkspaceState` no longer exists
- `WorkspaceCatalog` is catalog-only and domain-owned
- `WorkspaceSessionState` is application-internal
- `WorkspaceSnapshot` is the only read boundary for UI
- workspace ports live in `core/ports/workspace`
- writer ports use command structs instead of long primitive parameter lists
- domain entities are not used as UI read models
- `AliasValue` is removed and `AliasUsage` is either justified or removed
- analysis JSON fields no longer define the public model
