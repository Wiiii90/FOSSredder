# UI Source Testing Matrix

## Purpose

This document defines the current target test matrix for the `ui/src` C++
surface.

It mirrors the refactored UI boundary and keeps the source tests aligned with
the real code in `ui/tests`. The goal is to lock in the behavior of the UI
bridge layer:

- workspace facade and selection wiring
- session state and derived collections
- row projection and selection helpers
- mutation normalization helpers
- neutral navigation state
- thin adapter smoke checks
- small workflow smoke checks

The matrix is intentionally family-based. When the same rule applies to all
workspace collections, selection helpers, or row projection helpers, the test
should describe that family instead of a single incidental screen-specific
example.

## Scope

Included in this matrix:
- `ui/tests/unit/*`
- `ui/tests/interaction/*`
- `ui/tests/support/*`
- the source layer in `ui/include/ui/*` and `ui/src/*` that these tests cover

Out of scope:
- QML rendering and visual layout
- `core` domain and application behavior
- persistence and infrastructure adapters

## Target Test Tree

```text
ui/
  tests/
    support/
      FakeStorageManager.h
      WorkspaceTestData.h
    unit/
      TestAnalysisPayloadMapper.cpp
      TestAnnualState.cpp
      TestAnnualWorkflow.cpp
      TestAnalysisWorkflow.cpp
      TestBookingState.cpp
      TestExportState.cpp
      TestExportWorkflow.cpp
      TestImportSuggestionService.cpp
      TestImportWorkflow.cpp
      TestNavigationState.cpp
      TestSelectionState.cpp
      TestSettingsState.cpp
      TestSessionModels.cpp
      TestSessionMutationState.cpp
      TestWorkspaceFacade.cpp
      TestWorkspaceStartupRehydration.cpp
      TestWorkspaceRowProjector.cpp
      TestWorkspaceSessionSelection.cpp
      TestWorkspaceSessionState.cpp
    interaction/
      TestImportState.cpp
```

## Testing Principles

- Source tests should validate observable behavior, not private helper details.
- Workspace tests should verify projection, selection, and boundary routing.
- Session tests should verify that collections stay consistent across refresh
  and deletion impact.
- Projection refresh tests should verify that derived rows rebind when the
  workspace revision changes.
- Row projection tests should verify stable ids, stable ordering, and stable
  selection behavior across all workspace families.
- Ordered selection projection should expose a flat row contract for QML:
  `rows`, `orderIds`, `index`, `id`, and `currentId` should stay in sync with
  the nested `selection` payload.
- Adapter smoke tests should keep the payload surface and mapper headers
  usable, even when the implementation remains intentionally thin.
- Workflow smoke tests should keep the import and export boundaries usable
  without coupling the source tests to QML rendering.
- Prefer family-based assertions over screen-specific one-off cases when the
  same rule applies to multiple collections.

## 1. Workspace Boundary

`WorkspaceFacade` is the UI-facing entry point for workspace projection,
selection access, and core-boundary mutation routing.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| WSP-001 | Workspace projection loads all catalog families | Unit | Facade with a representative workspace catalog | Load the catalog into the facade | Actor, property, contract, statement, transaction, analysis, and annual rows are all available |
| WSP-002 | Selection accessors resolve current workspace rows | Unit | Facade with a representative workspace catalog | Set actor, property, contract, statement, transaction, analysis, and annual selection ids | The selected rows resolve to the expected objects |
| WSP-003 | Statement transaction ids stay readable through the facade | Unit | Facade with a representative statement | Query statement transaction ids | The facade returns the current ordered transaction id list |
| WSP-004 | Workspace path changes propagate through the boundary | Unit | Facade wired to a storage manager stub | Create or open a workspace path | The facade and storage manager agree on the current path |
| WSP-005 | Mutations route through the core boundary and refresh rows | Unit | Facade wired to a writable core boundary | Add and delete actors | The actor row model updates deterministically after each mutation |
| WSP-006 | Workspace revision changes rebind derived row projections | Unit | Facade with a representative workspace catalog and a visible derived row consumer | Mutate the workspace and bump the revision | Statement, transaction, and other derived collection bindings re-evaluate deterministically |
| WSP-007 | Startup rehydrates persisted workspace families into UI models | Unit | Facade attached to a loaded workspace session state with workflow logs | Open the workspace and refresh the facade state | Booking, analysis, import, and export rows are restored from persisted state without dropping relations |
| WSP-008 | Analysis update preserves calc adjustments | Unit | Facade attached to a workspace with an analysis containing calculation adjustments | Update analysis metadata with and without an explicit adjustments payload | Existing adjustments survive metadata-only updates, and explicit adjustment payloads replace the core analysis adjustments |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| WSP-C-001 | Workspace facade stays the public workspace entry point | Contract | UI workspace headers available | Review public API | The facade exposes workspace behavior without reintroducing old controller classes |
| WSP-C-002 | Workspace projection stays source-side | Contract | Workspace source files available | Review include graph | Projection helpers remain in the UI source layer and do not leak into core |

## 2. Session State

`SessionModels`, `SessionSelection`, `SelectionState`, `NavigationState`, and
`SessionMutationState` keep the UI workspace state deterministic. View-specific
state wrappers such as `BookingState` own deterministic screen behavior that is
too stateful for declarative QML.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ST-001 | Session models project every workspace collection family | Unit | Session loaded from a representative workspace catalog | Rebuild the models | Actor, property, contract, statement, transaction, analysis, and annual models all match the source catalog |
| ST-002 | Session models refresh derived transaction types after contract changes | Unit | Catalog with a changed contract type | Reload the models | Transaction types reflect the updated contract type |
| ST-003 | Session state exposes filtered transaction views for statements and properties | Unit | Session loaded from a representative workspace catalog | Query statement and property filters | Statement and property transaction filters return the expected counts |
| ST-004 | Session state applies deletion impact across collections and filters | Unit | Session loaded from a representative workspace catalog | Apply statement, transaction, and property deletion impact | The affected collections and filtered views are cleared consistently |
| ST-005 | Session state updates transaction property assignments in place | Unit | Session loaded from a representative workspace catalog | Reassign transaction property ids | The transaction row updates and the property filter shrinks accordingly |
| ST-006 | Session selection tracks current rows across all collections | Unit | Session models with a representative workspace catalog | Select actor, property, contract, statement, transaction, analysis, and annual ids | Each selected row resolves to the expected object |
| ST-007 | Session selection clears stale selections after reload | Unit | Session models with selected rows | Remove the backing rows and refresh selection | Stale ids and stale analysis result state are cleared deterministically |
| ST-008 | Selection state projects and clears rows consistently | Unit | Selection state with a representative workspace catalog | Set and then remove source rows | Selected rows resolve while present and clear once the source disappears |
| ST-009 | Navigation state stores the current section and settings category | Unit | Fresh navigation state | Change section and settings category values | Section and category values stay in sync with the enum representation |
| ST-010 | Mutation helpers normalize string and draft state families | Unit | Mixed string collections and draft payloads | Normalize, insert, remove, and current-state helpers | String collections, transaction drafts, and draft-list helpers stay deterministic |
| BKG-ST-001 | Booking create commit | Unit | Fresh workspace and BookingState draft | Fill statement and transaction fields and submit | A statement and transaction are created, selection moves to the created statement, and amount/status/allocatable values persist |
| BKG-ST-002 | Booking empty default transaction | Unit | Fresh workspace and BookingState draft with only statement name | Submit the statement | The statement is created without committing the empty default transaction draft |
| BKG-ST-003 | Booking incomplete transaction guard | Unit | Fresh workspace and BookingState draft with a statement name and incomplete transaction content | Attempt submit | Create remains disabled and no statement or transaction is committed |
| BKG-ST-004 | Booking multiple transaction drafts | Unit | Fresh workspace and BookingState draft with two complete transaction drafts | Submit the statement | Both transaction drafts are committed in order under the new statement |
| BKG-ST-005 | Booking amount text editing | Unit | Fresh workspace and BookingState draft | Edit a decimal amount text value | The draft preserves the decimal separator text while editing |
| BKG-ST-006 | Booking decimal amount commit | Unit | Fresh workspace and BookingState draft with decimal amount text | Submit the statement | The committed transaction amount keeps the decimal value instead of collapsing digits |
| BKG-ST-007 | Booking contract cascade | Unit | BookingState with actor, property, and contract rows | Select a contract row | The draft receives the contract id plus related actor and property ids |
| BKG-ST-008 | Booking incompatible actor cleanup | Unit | BookingState draft with a contract selected | Select an actor outside the selected contract | The actor id changes and the incompatible contract id is cleared |
| BKG-ST-009 | Booking incompatible property cleanup | Unit | BookingState draft with a contract selected | Select a property outside the selected contract | The property selection changes and the incompatible contract id is cleared |
| BKG-ST-010 | Booking edit update | Unit | Loaded statement and transaction selected | Change statement name and transaction fields, then update | Statement and transaction mutations persist and dirty state clears |
| ANL-ST-001 | Analysis include-calc preview refresh | Unit | AnalysisState loaded with a selected analysis containing calculation adjustments | Toggle include-calc off | Visible preview totals rebuild from the unadjusted analysis result |
| ANL-ST-002 | Analysis create with calc adjustments | Unit | AnalysisState in create mode with preview transactions and all filters selected | Select an adjustment transaction, enter a localized percent value, apply calc, and create | The new analysis persists adjustments, stores all-selection config filters as unfiltered, and the initial preview render uses the adjusted values |
| ANL-ST-003 | Analysis update preserves stored calc amounts | Unit | AnalysisState loaded with an existing analysis containing stored calculation adjustments | Update analysis metadata in edit mode | Existing adjustment amounts remain unchanged in the core snapshot and pending UI state |
| ANL-ST-004 | Analysis filter unassigned options | Unit | AnalysisState in create mode with workspace choices loaded | Inspect property and contract-type filter choices, then trigger the unassigned-only shortcuts | Both filter lists expose explicit unassigned choices and the shortcut selections keep the UI state deterministic |
| ANN-ST-001 | Annual state loads selected annual | Unit | AnnualState loaded with a selected annual and analysis rows | Refresh from selection | Form fields, assigned/available analysis rows, and derived transaction sections reflect the annual |
| ANN-ST-002 | Annual assignment preview and update | Unit | AnnualState loaded with one assigned and one available analysis | Add the available analysis and submit update | The preview updates, dirty state is set, and the annual persists both analysis ids |
| ANN-ST-003 | Annual create commit | Unit | AnnualState in create mode with analysis rows | Enter name/year, assign an analysis, and create | A new annual is persisted and selected through the workspace facade |
| ANN-ST-004 | Annual export format routing | Unit | AnnualState with an assigned plot analysis | Change the export format | The analysis update is routed through the workspace facade and dirty state reflects the annual metadata change |
| EXP-ST-001 | Export state loads workspace rows | Unit | ExportState with a representative workspace | Refresh state | Annual and analysis rows are available and the pending annual selection is deterministic |
| EXP-ST-002 | Export annual item projection | Unit | ExportState in annual mode | Add pending annual entry | Export items contain the annual plus assigned analysis items with default export formats |
| EXP-ST-003 | Export standalone analysis projection | Unit | ExportState in analysis mode with a plot analysis selected | Add pending analysis entry | The standalone analysis entry uses image export options and the default plot format |
| EXP-ST-004 | Export item restoration | Unit | ExportState receives persisted annual and analysis export items | Load items | Annual entries are rebuilt without duplicate assigned analyses and preserve explicit export formats |
| EXP-ST-005 | Export settings defaults | Unit | ExportState receives SettingsViewModel defaults | Change default export directory while the form still follows defaults, then after manual override | The pristine form refreshes immediately, while a manual target directory is preserved |
| SET-ST-001 | Settings category navigation | Unit | SettingsState connected to NavigationState | Navigate past both category edges | Category navigation wraps deterministically |
| SET-ST-002 | Settings category rows | Unit | SettingsState connected to NavigationState | Select a category and inspect rows | Category rows expose stable category values and selected state |
| SET-ST-003 | Settings file selections | Unit | SettingsState connected to SettingsViewModel and Actions | Emit selected import and export paths | The settings view model receives the selected default paths |
| SET-ST-004 | Settings property delegation | Unit | SettingsState connected to SettingsViewModel | Change export, import, and toolbar properties | Setters delegate to the settings view model |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ST-C-001 | Session state remains UI-local | Contract | Session source files available | Review include graph | Session logic stays inside the UI source layer |
| ST-C-002 | Mutation logic stays reusable across collections | Contract | Mutation helper source available | Review helper surface | Shared mutation helpers are used instead of duplicating collection-specific logic |

## 3. Row Projection And Adapters

`WorkspaceRowProjector` and the thin mapper smoke tests protect the row and
payload shapes that QML and workflows consume.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ADP-001 | Workspace row projection stays stable for all workspace families | Unit | Session store loaded from a representative catalog | Build actor, property, contract, analysis, annual, statement, and transaction rows | Each family exposes deterministic ids, names, and role payloads |
| ADP-002 | Workspace row ordering preserves the generic selection contract | Unit | Synthetic row list with ordered ids | Insert, prune, and reorder ids | Ordering, insertion, wrapping, and reselection behave deterministically, and the ordered selection payload exposes stable `orderIds`, `index`, `id`, and `currentId` fields |
| ADP-003 | Analysis payload mapper header stays usable | Unit | Mapper header available | Include the mapper header | The adapter remains a valid build-time contract surface |
| ADP-004 | Import suggestion mapper header stays usable | Unit | Mapper header available | Include the mapper header | The adapter remains a valid build-time contract surface |
| ADP-005 | Analysis preview returns filtered transactions from the current workspace snapshot | Unit | Analysis workflow with a representative workspace catalog | Request a preview for an empty filter and for a matching filter expression | The preview returns the visible transactions, the count metrics, and the row payloads stay aligned with the workspace snapshot |
| ADP-006 | Analysis filter composition defaults to the previous year and skips unrestricted groups | Unit | Analysis workflow with UI filter inputs and settings defaults | Build a filter from an unconfigured year selector and from all-selected property and contract groups | The canonical filter uses the previous year as default, keeps the year mode as the default case, and omits property and contract clauses when the selection is unrestricted |
| ADP-007 | Analysis filter composition keeps explicit partial selections intact | Unit | Analysis workflow with UI filter inputs | Build a filter from a partial property selection, a partial contract selection, and an explicit allocatable mode | The canonical filter preserves the explicit clauses and remains stable for preview and execution |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ADP-C-001 | Row projector stays a UI source helper | Contract | Adapter headers available | Review public API | Row projection helpers stay capability-based and do not drift back into controller naming |
| ADP-C-002 | Thin mappers stay header-usable | Contract | Mapper headers available | Compile the smoke tests | The mapper headers remain included and usable without runtime coupling |

## 4. Workflow Smokes

The workflow unit tests keep the export and import boundaries usable without
binding the suite to QML rendering.

### ExportWorkflow

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-W-001 | Restored export runs are projected from persisted state | Unit | Snapshot provider with export logs | Construct the workflow | Restored export runs are visible in the workflow model |
| EXP-W-002 | Snapshot refresh replaces export run rows deterministically | Unit | Workflow with restored runs and a changed snapshot | Refresh from the state snapshot | Workflow rows rebind to the new snapshot content |
| EXP-W-003 | Log-store hookup does not overwrite restored export runs | Unit | Workflow with restored runs and a writable log store | Bind the log store and mutate a run | Restored rows remain intact until a user mutation occurs |

### ImportWorkflow

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-W-001 | Restored import runs are projected from persisted state | Unit | Snapshot provider with import logs | Construct the workflow | Restored import runs are visible in the workflow model |
| IMP-W-002 | Snapshot refresh replaces import run rows deterministically | Unit | Workflow with restored runs and a changed snapshot | Refresh from the state snapshot | Workflow rows rebind to the new snapshot content |
| IMP-W-003 | Log-store hookup does not overwrite restored import runs | Unit | Workflow with restored runs and a writable log store | Bind the log store and mutate a run | Restored rows remain intact until a user mutation occurs |
| IMP-W-004 | Import pause progress gate | Unit | Running import workflow state | Toggle pause and resume | Progress updates are ignored while paused and accepted after resume |
| IMP-W-005 | Legacy draft-log restoration | Unit | Snapshot provider with a Draft import log carrying `statementDraftIds` but no attached-draft flag | Restore import runs | The workflow projects a clickable draft row with the restored draft id |
| IMP-W-006 | Draft stack navigation cycles through import home | Unit | Snapshot provider with multiple persisted statement drafts and draft logs | Navigate next from the last draft or previous from the first draft, then navigate again | The active draft clears to the import home slot first, then continues to the opposite edge |
| IMP-W-007 | Statement draft transaction navigation wraps | Unit | Statement draft view-model with multiple transactions | Navigate next from the last transaction and previous from the first transaction | The active transaction index wraps to the opposite edge |
| IMP-W-008 | Reopened draft restores remembered transaction | Unit | Persisted draft is opened after selecting a non-first transaction | Clear and reopen the same draft | The draft opens with the previously selected transaction index |
| IMP-W-009 | Persisted draft positions follow current transaction order | Unit | Statement draft contains manually reordered or inserted transactions with stale position fields | Persist the draft snapshot | Transaction draft snapshot positions are rewritten from the current list order |
| IMP-W-010 | Actor quick-create selection helper updates current transaction state | Unit | Import workflow with a draft and actor row provider | Create/select actor for current draft via workflow helper | Current transaction actor id/text/selection flags update deterministically and draft refresh is requested |
| IMP-W-011 | Contract selection helper clears incompatible actor and property overrides | Unit | Import workflow with selected contract and existing manual actor/property selection | Select a contract row for current transaction | Contract id is set and dependent actor/property fields are normalized for consistency |
| IMP-W-012 | Property or actor manual selection clears incompatible contract | Unit | Import workflow with current contract selected | Select actor or property that is incompatible with selected contract | Contract selection clears to no-contract and dependent derived fields re-evaluate deterministically |

### AnnualWorkflow

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-W-001 | Stored annual computation | Unit | Annual workflow with a representative workspace snapshot | Compute an existing annual | Stored annual assignments are passed to the annual application service and category payloads are returned |
| ANN-W-002 | Annual preview computation | Unit | Annual workflow with a representative workspace snapshot | Compute a preview with injected analysis ids and year | The preview uses the injected annual selection without mutating the workspace source |

## 5. Import Interaction

The interaction smoke test keeps the import workflow boundary usable without
binding the suite to QML rendering.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-001 | Import workflow header remains usable | Interaction | Import workflow state header available | Include the workflow state header | The import workflow boundary remains buildable and accessible from the UI source layer |
| IMP-002 | Import overview state selection | Interaction | ImportState with ImportWorkflow and SettingsViewModel | Apply and change default PDF path, then commit manual non-PDF and PDF paths | Default PDF changes refresh the pristine selection, non-PDF input is ignored, and a manual PDF selection is not overwritten by later default changes |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-C-001 | Import state remains workflow-local | Contract | Import workflow files available | Review include graph | Import state does not leak into unrelated UI source modules |

## Test File Guidance

Recommended file layout:

```text
ui/
  tests/
    support/
      FakeStorageManager.h
      WorkspaceTestData.h
    unit/
      TestAnalysisPayloadMapper.cpp
      TestAnalysisWorkflow.cpp
      TestAnnualState.cpp
      TestAnnualWorkflow.cpp
      TestExportState.cpp
      TestExportWorkflow.cpp
      TestImportSuggestionService.cpp
      TestImportWorkflow.cpp
      TestNavigationState.cpp
      TestSelectionState.cpp
      TestSettingsState.cpp
      TestSessionModels.cpp
      TestSessionMutationState.cpp
      TestWorkspaceFacade.cpp
      TestWorkspaceRowProjector.cpp
      TestWorkspaceSessionSelection.cpp
      TestWorkspaceSessionState.cpp
    interaction/
      TestImportState.cpp
```

## Implementation Order

1. Workspace facade and session projection.
2. Session selection, navigation, and mutation helpers.
3. Row projection and mapper smoke checks.
4. Analysis workflow filter composition and preview checks.
5. Export and import workflow smoke checks.
6. Import interaction smoke coverage.

## Definition of Done

The `ui/src` test suite is complete when:

- the workspace facade boundary is covered by behavior tests
- session models, selection, navigation, and mutation state are covered by
  stable family-based tests
- row projection helpers are covered for every workspace family
- mapper smoke tests remain buildable and usable
- export and import workflow smoke tests remain buildable and usable
- analysis filter composition and preview behavior stay stable across default and partial-selection cases
- the import workflow boundary remains smoke-tested
- the document tree matches the actual `ui/tests` tree
