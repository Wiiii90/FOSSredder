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
        TestWorkspaceSelection.cpp
        TestWorkspaceSelectors.cpp
        TestWorkspaceStore.cpp
      shell/
        TestNavigation.cpp
    interaction/
      TestFeatureWiring.cpp
      TestImportState.cpp
```

The current unit tree is the active `ui/src` source test suite. QML rendering
tests live in the separate QML matrix and are intentionally not duplicated here.
Interaction tests cover small cross-layer smokes that do not render QML.

## Layer Responsibilities

| Layer | Test intent |
|---|---|
| `support` | Shared fakes, sample data, and harnesses for UI source tests. |
| `unit/viewmodels` | Public QML API entries: setters, invokables, emitted state, validation, and routing to workflow/workspace ports. |
| `unit/workflows` | Feature orchestration that is too async or multi-step for a ViewModel-only test. |
| `unit/adapters` | DTO and port-boundary translation. These tests should mock core ports and avoid workspace/QML concerns. |
| `unit/workspace` | WorkspaceStore, WorkspaceCommands, WorkspaceSelection, WorkspaceSelectors, and WorkspacePayloads. |
| `unit/shell` | Small shell objects that are not feature ViewModels. |
| `interaction` | Cross-layer ViewModel, Workflow, Adapter, and Workspace wiring without QML rendering. |

## ViewModel Matrix

| ID | File | Scope |
|---|---|---|
| VM-ACTOR-001 | `TestActorViewModel.cpp` | Create actor and select the persisted row. |
| VM-ACTOR-002 | `TestActorViewModel.cpp` | Select, edit, update, and delete the current actor. |
| VM-PROPERTY-001 | `TestPropertyViewModel.cpp` | Create property and persist contract assignments. |
| VM-PROPERTY-002 | `TestPropertyViewModel.cpp` | Select, edit, update, and delete the current property. |
| VM-CONTRACT-001 | `TestContractViewModel.cpp` | Create contract with actor, property, and allocatable mode. |
| VM-CONTRACT-002 | `TestContractViewModel.cpp` | Select, edit, update, and delete the current contract. |
| VM-BOOKING-001 | `TestBookingViewModel.cpp` | Create a statement and transaction through the booking API. |
| VM-BOOKING-002 | `TestBookingViewModel.cpp` | Update current statement and transaction fields. |
| VM-IMPORT-001 | `TestImportViewModel.cpp` | Queue only supported PDF files from editable import input. |
| VM-IMPORT-002 | `TestImportViewModel.cpp` | Open a draft import log and switch to draft content/navigation. |
| VM-IMPORT-003 | `TestImportViewModel.cpp` | Rename current draft and navigate transaction drafts. |
| VM-IMPORT-004 | `TestImportViewModel.cpp` | Add and delete transaction drafts through the adapter. |
| VM-IMPORT-005 | `TestImportViewModel.cpp` | Commit editable transaction text into the current draft state. |
| VM-IMPORT-006 | `TestImportViewModel.cpp` | Update property selection for the current transaction draft. |
| VM-ANALYSIS-001 | `TestAnalysisViewModel.cpp` | Toggle calculation adjustments and refresh visible preview state. |
| VM-ANALYSIS-002 | `TestAnalysisViewModel.cpp` | Create analysis and persist adjustments for the initial preview. |
| VM-ANALYSIS-003 | `TestAnalysisViewModel.cpp` | Update analysis while preserving stored adjustment amounts. |
| VM-ANALYSIS-004 | `TestAnalysisViewModel.cpp` | Expose unassigned/none filter options and submit gating. |
| VM-ANNUAL-001 | `TestAnnualViewModel.cpp` | Load selected annual and derived transaction panels. |
| VM-ANNUAL-002 | `TestAnnualViewModel.cpp` | Update assignments, preview, and persisted annual state. |
| VM-ANNUAL-003 | `TestAnnualViewModel.cpp` | Create a new annual through workspace commands. |
| VM-ANNUAL-004 | `TestAnnualViewModel.cpp` | Route analysis export format changes through workspace. |
| VM-EXPORT-001 | `TestExportViewModel.cpp` | Load workspace rows and select annual export mode. |
| VM-EXPORT-002 | `TestExportViewModel.cpp` | Project assigned analyses from an annual export entry. |
| VM-EXPORT-003 | `TestExportViewModel.cpp` | Default standalone plot entries to image export options. |
| VM-EXPORT-004 | `TestExportViewModel.cpp` | Refresh pristine export form from settings defaults. |
| VM-SETTINGS-001 | `TestSettingsViewModel.cpp` | Wrap category navigation through shell navigation. |
| VM-SETTINGS-002 | `TestSettingsViewModel.cpp` | Mark the selected settings category row. |
| VM-SETTINGS-003 | `TestSettingsViewModel.cpp` | Apply file-selection signals to settings values. |
| VM-SETTINGS-004 | `TestSettingsViewModel.cpp` | Delegate settings properties to the settings store. |

## Workflow Matrix

| ID | File | Scope |
|---|---|---|
| WF-IMPORT-001 | `TestImportWorkflow.cpp` | Cycle draft-stack navigation through the import home view. |
| WF-IMPORT-002 | `TestImportWorkflow.cpp` | Restore remembered transaction index when reopening a draft. |
| WF-IMPORT-003 | `TestImportWorkflow.cpp` | Select first added file and deduplicate queued files. |
| WF-IMPORT-004 | `TestImportWorkflow.cpp` | Flush active draft to a workspace draft snapshot. |
| WF-IMPORT-005 | `TestImportWorkflow.cpp` | Finalize active draft through workspace commands. |
| WF-EXPORT-001 | `TestExportWorkflow.cpp` | Publish export logs through the workspace sink. |
| WF-EXPORT-002 | `TestExportWorkflow.cpp` | Publish success log and reset state after a successful export. |
| WF-EXPORT-003 | `TestExportWorkflow.cpp` | Propagate cancel requests and publish canceled logs. |
| WF-EXPORT-004 | `TestExportWorkflow.cpp` | Update workflow state for pause and resume. |
| WF-ANALYSIS-001 | `TestAnalysisWorkflow.cpp` | Return all matching workspace transactions for previews. |
| WF-ANALYSIS-002 | `TestAnalysisWorkflow.cpp` | Honor analysis filters in transaction previews. |
| WF-ANALYSIS-003 | `TestAnalysisWorkflow.cpp` | Use year defaults and omit fully selected groups in filter specs. |
| WF-ANALYSIS-004 | `TestAnalysisWorkflow.cpp` | Emit explicit filter clauses for partial selections. |
| WF-ANALYSIS-005 | `TestAnalysisWorkflow.cpp` | Store adjusted amounts by transaction id. |
| WF-ANALYSIS-006 | `TestAnalysisWorkflow.cpp` | Persist analysis snapshots and adjustments through workspace commands. |
| WF-ANALYSIS-007 | `TestAnalysisWorkflow.cpp` | Carry stored adjustments into plot previews. |
| WF-ANALYSIS-008 | `TestAnalysisWorkflow.cpp` | Include projected table state in computed previews. |
| WF-ANNUAL-001 | `TestAnnualWorkflow.cpp` | Compute annual preview from stored annual assignments. |
| WF-ANNUAL-002 | `TestAnnualWorkflow.cpp` | Use injected analysis ids and year for preview computation. |
| WF-ANNUAL-003 | `TestAnnualWorkflow.cpp` | Save and delete annuals through workspace commands. |
| WF-ANNUAL-004 | `TestAnnualWorkflow.cpp` | Preserve analysis payload when export format changes. |

## Adapter Matrix

| ID | File | Scope |
|---|---|---|
| ADP-IMPORT-001 | `TestImportAdapter.cpp` | Delegate property selection to `IImportRunner` and normalize draft state. |
| ADP-EXPORT-001 | `TestExportAdapter.cpp` | Build export requests from package and object selection. |
| ADP-EXPORT-002 | `TestExportAdapter.cpp` | Delegate export execution to the export runner. |
| ADP-ANALYSIS-001 | `TestAnalysisAdapter.cpp` | Delegate analysis run and preview calls to the analysis runner. |
| ADP-ANALYSIS-002 | `TestAnalysisAdapter.cpp` | Map analysis results to QML payload shape. |
| ADP-ANALYSIS-003 | `TestAnalysisAdapter.cpp` | Build analysis requests by parsing filters through the runner. |
| ADP-ANNUAL-001 | `TestAnnualAdapter.cpp` | Delegate annual computation to the annual runner. |
| ADP-ANNUAL-002 | `TestAnnualAdapter.cpp` | Map annual results to QML payload shape. |
| ADP-ANNUAL-003 | `TestAnnualAdapter.cpp` | Map missing-live annual buckets from core results. |

## Workspace Matrix

| ID | File | Scope |
|---|---|---|
| WSP-COMMANDS-001 | `TestWorkspaceCommands.cpp` | Route actor save through writer and refresh the store. |
| WSP-COMMANDS-002 | `TestWorkspaceCommands.cpp` | Persist import and export logs through workspace commands. |
| WSP-COMMANDS-003 | `TestWorkspaceCommands.cpp` | Run the pre-save callback before storage save. |
| WSP-SELECTION-001 | `TestWorkspaceSelection.cpp` | Track current selection across all workspace collections. |
| WSP-SELECTION-002 | `TestWorkspaceSelection.cpp` | Clear stale selections after workspace reload. |
| WSP-SELECTORS-001 | `TestWorkspaceSelectors.cpp` | Project catalog rows and lookup helpers. |
| WSP-SELECTORS-002 | `TestWorkspaceSelectors.cpp` | Project statement transactions and transaction rows. |
| WSP-SELECTORS-003 | `TestWorkspaceSelectors.cpp` | Delegate catalog selection rules to the workspace reader. |
| WSP-STORE-001 | `TestWorkspaceStore.cpp` | Load snapshots and track data revisions. |
| WSP-STORE-002 | `TestWorkspaceStore.cpp` | Apply deletion impacts across related collections. |
| WSP-STORE-003 | `TestWorkspaceStore.cpp` | Resolve current path through the bound reader. |

## Shell Matrix

| ID | File | Scope |
|---|---|---|
| SHELL-NAVIGATION-001 | `TestNavigation.cpp` | Store and expose current section and settings category. |
| SHELL-NAVIGATION-002 | `TestNavigation.cpp` | Expose QML section constants and navigate by value. |

## Interaction Matrix

| ID | File | Scope |
|---|---|---|
| INTERACTION-IMPORT-001 | `TestImportState.cpp` | Apply default import path and filter manual files. |
| INTERACTION-IMPORT-002 | `TestImportState.cpp` | Gate workflow progress updates while paused. |
| INTERACTION-IMPORT-003 | `TestImportState.cpp` | Finalize draft through ViewModel, workflow, and workspace. |
| INTERACTION-ANALYSIS-001 | `TestFeatureWiring.cpp` | Create analysis through ViewModel, workflow, and workspace. |
| INTERACTION-ANNUAL-001 | `TestFeatureWiring.cpp` | Create annual through ViewModel, workflow, and workspace. |
| INTERACTION-EXPORT-001 | `TestFeatureWiring.cpp` | Run export through ViewModel, workflow, adapter, and log sink. |

## Current Unit Test Coverage

| File | Test count |
|---|---:|
| `unit/adapters/TestAnalysisAdapter.cpp` | 3 |
| `unit/adapters/TestAnnualAdapter.cpp` | 3 |
| `unit/adapters/TestExportAdapter.cpp` | 2 |
| `unit/adapters/TestImportAdapter.cpp` | 1 |
| `unit/shell/TestNavigation.cpp` | 2 |
| `unit/viewmodels/TestActorViewModel.cpp` | 2 |
| `unit/viewmodels/TestAnalysisViewModel.cpp` | 4 |
| `unit/viewmodels/TestAnnualViewModel.cpp` | 4 |
| `unit/viewmodels/TestBookingViewModel.cpp` | 2 |
| `unit/viewmodels/TestContractViewModel.cpp` | 2 |
| `unit/viewmodels/TestExportViewModel.cpp` | 4 |
| `unit/viewmodels/TestImportViewModel.cpp` | 6 |
| `unit/viewmodels/TestPropertyViewModel.cpp` | 2 |
| `unit/viewmodels/TestSettingsViewModel.cpp` | 4 |
| `unit/workflows/TestAnalysisWorkflow.cpp` | 8 |
| `unit/workflows/TestAnnualWorkflow.cpp` | 4 |
| `unit/workflows/TestExportWorkflow.cpp` | 4 |
| `unit/workflows/TestImportWorkflow.cpp` | 5 |
| `unit/workspace/TestWorkspaceCommands.cpp` | 3 |
| `unit/workspace/TestWorkspaceSelection.cpp` | 2 |
| `unit/workspace/TestWorkspaceSelectors.cpp` | 3 |
| `unit/workspace/TestWorkspaceStore.cpp` | 3 |

Total: 22 unit test files, 73 test cases.

## Current Interaction Test Coverage

| File | Test count |
|---|---:|
| `interaction/TestFeatureWiring.cpp` | 3 |
| `interaction/TestImportState.cpp` | 3 |

Total: 2 interaction test files, 6 test cases.

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
off in debug sessions with `FOSSREDDER_UI_TRACE=0`.

## Testing Principles

- Test observable ViewModel API behavior before private helpers.
- Mock core ports at adapter/workflow boundaries.
- Use `support/ViewModelTestHarness.h` for common workspace role wiring.
- Put shared runner stubs and UI port fakes in `support` once two tests use
  them.
- Keep QML tests for rendering and source tests for behavior.
- Prefer focused, family-based assertions where the same workspace rule applies
  to multiple collections.
- Avoid placeholder tests that only instantiate a class without asserting a real
  contract.

## Implementation Order

1. Keep the existing workspace, workflow, adapter, shell, and ViewModel tests
   green.
2. Move repeated local fakes into `support` only when at least two tests share
   them.
3. Add new source tests only when they cover a public ViewModel command path,
   workflow orchestration contract, adapter boundary, or workspace/state role.

## Definition of Done

The `ui/src` suite is complete when every public ViewModel command path that can
change state is covered, every workflow has focused orchestration tests, every
adapter has a mocked port-boundary test, and ViewModels only consume the
concrete workspace roles they need: Store, Commands, Selection, Selectors, and
Payloads.
