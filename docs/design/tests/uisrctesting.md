# UI Source Testing Matrix

## Purpose

This document defines the target test structure for the C++ UI bridge in
`ui/src`.

The main behavioral surface is the QML-facing ViewModel API. Most user actions
enter through ViewModels, then route through workspace roles, workflows, and
adapters into core ports. Tests should therefore mirror that shape instead of
rebuilding screen logic in arbitrary files.

## Scope

Included:
- `ui/tests/unit/*`
- `ui/tests/interaction/*`
- `ui/tests/support/*`
- `ui/include/ui/*` and `ui/src/*`

Out of scope:
- QML visual rendering and layout
- core domain behavior
- persistence and infrastructure implementation details

## Target Test Tree

```text
ui/
  tests/
    support/
      ImportRunnerStub.h
      ViewModelTestHarness.h
      WorkspacePortFakes.h
      WorkspaceTestData.h
    unit/
      viewmodels/
        TestActorViewModel.cpp
        TestAnalysisViewModel.cpp
        TestAnnualViewModel.cpp
        TestBookingViewModel.cpp
        TestContractViewModel.cpp
        TestExportViewModel.cpp
        TestImportViewModel.cpp
        TestPropertyViewModel.cpp
        TestSettingsViewModel.cpp
      workflows/
        TestAnalysisWorkflow.cpp
        TestAnnualWorkflow.cpp
        TestExportWorkflow.cpp
        TestImportWorkflow.cpp
      adapters/
        TestAnalysisAdapter.cpp
        TestAnnualAdapter.cpp
        TestExportAdapter.cpp
        TestImportAdapter.cpp
      workspace/
        TestWorkspaceCommands.cpp
        TestWorkspacePayloads.cpp
        TestWorkspaceSelection.cpp
        TestWorkspaceSelectors.cpp
        TestWorkspaceStore.cpp
      state/
        TestNavigationState.cpp
    interaction/
      TestFeatureWiring.cpp
      TestImportState.cpp
```

The current tree already uses the layer folders. Missing ViewModel and adapter
files are planned coverage, not placeholders that must exist before the tested
surface is meaningful.

## Layer Responsibilities

| Layer | Test intent |
|---|---|
| `support` | Shared fakes, sample data, and harnesses for UI source tests. |
| `unit/viewmodels` | Public QML API entries: setters, invokables, emitted state, validation, and routing to workflow/workspace ports. |
| `unit/workflows` | Feature orchestration that is too async or multi-step for a ViewModel-only test. |
| `unit/adapters` | DTO and port-boundary translation. These tests should mock core ports and avoid workspace/QML concerns. |
| `unit/workspace` | WorkspaceStore, WorkspaceCommands, WorkspaceSelection, WorkspaceSelectors, and WorkspacePayloads. |
| `unit/state` | Small UI-local state objects that are not feature ViewModels. |
| `interaction` | Cross-object interaction smokes that need real UI wiring but no QML rendering. |

Interaction tests should stay small. Each test should exercise one user-facing
flow across at least two UI source layers, such as ViewModel plus Workflow plus
Adapter, and should avoid duplicating detailed unit assertions.

## ViewModel Matrix

| ID | File | Scope |
|---|---|---|
| VM-ACTOR | `TestActorViewModel.cpp` | Create, update, delete, alias handling, contract selection routing, dirty state. |
| VM-PROPERTY | `TestPropertyViewModel.cpp` | Create, update, delete, alias handling, contract selection routing, dirty state. |
| VM-CONTRACT | `TestContractViewModel.cpp` | Create, update, delete, actor/property assignment, allocatable mode, incompatible selection cleanup. |
| VM-BOOKING | `TestBookingViewModel.cpp` | Statement and transaction creation/editing, amount parsing, selection, delete operations. |
| VM-IMPORT | `TestImportViewModel.cpp` | Import command API, file selection, run log projection, draft fields, draft transaction edits, and draft navigation. |
| VM-ANALYSIS | `TestAnalysisViewModel.cpp` | Filter composition, preview, create/update/delete, calculation adjustments, export settings. |
| VM-ANNUAL | `TestAnnualViewModel.cpp` | Annual assignment, preview, create/update/delete, export-format routing. |
| VM-EXPORT | `TestExportViewModel.cpp` | Export item construction, restored runs, settings defaults, delete/clear logs. |
| VM-SETTINGS | `TestSettingsViewModel.cpp` | Category navigation, persisted settings delegation, file-selection updates. |

## Workflow Matrix

| ID | File | Scope |
|---|---|---|
| WF-IMPORT | `TestImportWorkflow.cpp` | Import runner orchestration, draft persistence, run logs, pause/resume/cancel state. |
| WF-EXPORT | `TestExportWorkflow.cpp` | Export runner orchestration, restored runs, snapshot refresh, log store behavior. |
| WF-ANALYSIS | `TestAnalysisWorkflow.cpp` | Analysis preview/run coordination, filter defaults, table projection, snapshot JSON, and adjustment helpers. |
| WF-ANNUAL | `TestAnnualWorkflow.cpp` | Annual preview/run coordination and assigned analysis input. |

## Adapter Matrix

| ID | File | Scope |
|---|---|---|
| ADP-IMPORT | `TestImportAdapter.cpp` | Import DTO mapping, draft matcher/derived-state delegation via `IImportRunner`. |
| ADP-EXPORT | `TestExportAdapter.cpp` | Export request/log payload mapping and runner delegation. |
| ADP-ANALYSIS | `TestAnalysisAdapter.cpp` | Analysis request/result mapping and runner delegation. |
| ADP-ANNUAL | `TestAnnualAdapter.cpp` | Annual request/result mapping and runner delegation. |

## Workspace Matrix

| ID | Scope | Expected |
|---|---|---|
| WSP-001 | Workspace snapshot loading | All catalog families, logs, drafts, analyses, and annuals appear in UI models. |
| WSP-002 | Workspace mutations | Add/update/delete operations route through workspace writer ports and refresh projected rows. |
| WSP-003 | Selection | Actor, property, contract, statement, transaction, analysis, and annual selection stay deterministic after reload/deletion. |
| WSP-004 | Selectors and payloads | Rows expose stable ids, names, ordering, and QML payload keys without a parallel ListModel path. |
| WSP-005 | Store rehydration | Persisted workflow logs and workspace families restore without dropping relations. |
| WSP-006 | Storage commands | New/open/save/save-as route to writer ports and emit success/failure operations. |

## Interaction Matrix

| ID | File | Scope |
|---|---|---|
| INTERACTION-IMPORT | `TestImportState.cpp` | Import overview defaults, manual file filtering, and pause-state gating. |
| INTERACTION-ANALYSIS | `TestFeatureWiring.cpp` | AnalysisViewModel creates an analysis through workflow and workspace wiring. |
| INTERACTION-ANNUAL | `TestFeatureWiring.cpp` | AnnualViewModel creates an annual through workflow and workspace wiring. |
| INTERACTION-EXPORT | `TestFeatureWiring.cpp` | ExportViewModel starts an export through workflow, adapter, runner, and log sink. |

## Debug Trace Expectations

UI trace is a debug-only console channel in `ui/observability/Trace`.

Trace should be used for:
- ViewModel command entry points that represent user actions.
- Workflow state transitions and runner boundary events.
- Adapter calls when a UI request crosses into a core port.
- WorkspaceCommands mutations and file operations.

Trace should not be used for:
- simple property getters
- every model row read
- release builds
- replacing assertions in tests

The channel is compiled as a no-op outside debug-style builds and can be turned
on in debug sessions with `FOSSREDDER_UI_TRACE=1`.

## Testing Principles

- Test observable ViewModel API behavior before private helpers.
- Mock core ports at adapter/workflow boundaries.
- Use `support/ViewModelTestHarness.h` for common workspace role wiring.
- Put shared runner stubs and UI port fakes in `support` once two tests use
  them.
- Keep QML tests for rendering and source tests for behavior.
- Keep `interaction` as a thin wiring smoke suite; detailed behavior belongs in
  `unit`.
- Prefer focused, family-based assertions where the same workspace rule applies
  to multiple collections.
- Avoid placeholder tests that only instantiate a class without asserting a real
  contract.

## Implementation Order

1. Keep the existing workspace, workflow, and migrated ViewModel tests green.
2. Add missing `Test*ViewModel.cpp` files for Actor, Property, Contract, and
   Booking.
3. Add adapter tests for Import, Export, Analysis, and Annual.
4. Move repeated local fakes into `support` only when at least two tests share
   them.
5. Remove obsolete state naming from test classes once the ViewModel files are
   fully migrated internally.

## Definition of Done

The `ui/src` suite is complete when every public ViewModel command path is
covered, every workflow has focused orchestration tests, every adapter has a
mocked port-boundary test, and ViewModels only consume the concrete workspace
roles they need: Store, Commands, Selection, Selectors, and Payloads.
