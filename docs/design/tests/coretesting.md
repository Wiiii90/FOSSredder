# Core Testing Matrix

## Purpose

This document defines the target test matrix for the `core` module.
It mirrors the structure of the other testing docs in `docs/design/tests`, but
is tailored to the refactored core boundary:

- `core/domain` is tested as the pure domain model
- `core/application` is tested as orchestration and projection
- `core/ports` is tested as contract surface

The goal is not to test implementation accidents. The goal is to lock in the
behavioral contract of the core after the workspace and analysis refactors.

The suite is intentionally family-based: where a rule applies to all entities,
all collections, or all port boundaries, the test should describe that family
instead of a single incidental example.

## Scope

Included in this matrix:
- `core/include/core/domain/*`
- `core/src/domain/*`
- `core/include/core/application/*`
- `core/src/application/*`
- `core/include/core/ports/*`

Out of scope:
- `ui` behavior tests
- persistence schema and SQLite repository behavior
- infra adapters such as OCR, PDF, image processing, archive handling

## Target Test Tree

```text
core/
  tests/
    domain/
      catalog/
        TestWorkspaceCatalog.cpp
      entities/
        TestActor.cpp
        TestProperty.cpp
        TestContract.cpp
        TestStatement.cpp
        TestTransaction.cpp
        TestAnalysis.cpp
        TestAnnual.cpp
      values/
        TestAlias.cpp
        TestEntityName.cpp
        TestContractType.cpp
        TestAnalysisType.cpp
        TestExportFormat.cpp
        TestFilterSpec.cpp
        TestBookingDate.cpp
        TestMoneyAmount.cpp
        TestYear.cpp
      policies/
        TestAliasPolicy.cpp
        TestTransactionPolicy.cpp
        TestAnalysisPolicy.cpp
        TestAnnualPolicy.cpp
        TestStatementPolicy.cpp
        TestDraftMatchingPolicy.cpp
    application/
      analysis/
        TestAnalysisService.cpp
      annual/
        TestAnnualService.cpp
      workspace/
        TestWorkspaceCommandService.cpp
        TestWorkspaceFacade.cpp
        TestWorkspaceQueryService.cpp
        TestWorkspaceSnapshotProjector.cpp
        TestWorkspaceStateManager.cpp
        TestWorkspaceWorkflowService.cpp
      import/
        draft/
          TestDraftFinalizer.cpp
          TestDraftMatcher.cpp
          TestDraftMatcherProjection.cpp
    ports/
      workspace/
        TestWorkspaceReaderContract.cpp
        TestWorkspaceWriterContract.cpp
```

## Testing Principles

- Domain tests should validate invariants, behavior, and aggregate ownership.
- Application tests should validate orchestration, projections, and workflow
  boundaries.
- Port tests should validate contract shape and boundary semantics.
- Prefer behavior families over one-off examples when the same rule applies to
  actors, properties, contracts, statements, transactions, analyses, or annuals.
- Tests should stay close to the public API, not private helper internals.
- If a test needs a large fixture, prefer a reusable helper builder over a giant
  inline setup block.

## Domain

The domain suite is split into entities, values, policies, and the workspace
catalog aggregate.

### 1. WorkspaceCatalog

`WorkspaceCatalog` is the aggregate root for catalog-level invariants.

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CAT-001 | Add actor to catalog | Unit | Empty catalog | Call `addActor(...)` through the catalog service or aggregate helper | Actor is inserted and id is returned |
| CAT-002 | Delete statement removes owned transactions | Unit | Catalog with statement and linked transactions | Delete the statement | Statement and its owned transaction links are removed consistently |
| CAT-003 | Transaction cannot point to missing statement | Unit | Catalog without target statement | Link transaction to statement | Operation is rejected |
| CAT-004 | Analysis removal updates annual references | Unit | Catalog with annuals referencing analysis ids | Remove analysis | Annual references are cleaned up |
| CAT-005 | Alias normalization stays stable in catalog updates | Unit | Entity with mixed alias source/value pairs | Rehydrate or normalize | Alias values are canonicalized deterministically |
| CAT-006 | Catalog reports empty state correctly | Unit | Empty catalog | Call `empty()` | Returns `true` |

### 2. Entities

#### Actor

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-001 | Renaming actor updates name only | Unit | Actor with existing name | Call `rename(...)` | Name changes and other fields remain stable |
| ACT-002 | Setting aliases deduplicates canonical alias values | Unit | Actor with duplicate aliases | Call `setAliases(...)` | Alias list is normalized and deduplicated |
| ACT-003 | Alias usage stays consistent with alias set | Unit | Actor with alias usage data | Update aliases | Usage stays valid for retained aliases |
| ACT-004 | Created and updated timestamps are preserved through mutation helpers | Unit | Actor with timestamps | Mutate actor | Timestamps update according to the mutation rule |

#### Property

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-001 | Renaming property updates name only | Unit | Property with existing name | Call `rename(...)` | Name changes and other fields remain stable |
| PROP-002 | Setting aliases deduplicates canonical alias values | Unit | Property with duplicate aliases | Call `setAliases(...)` | Alias list is normalized and deduplicated |
| PROP-003 | Alias usage stays consistent with alias set | Unit | Property with alias usage data | Update aliases | Usage stays valid for retained aliases |

#### Contract

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-001 | Contract type validation blocks invalid type | Unit | Contract with invalid type | Call `setType(...)` or construct invalid input | Invalid type is rejected or normalized consistently |
| CON-002 | Actor ids are deduplicated | Unit | Contract with duplicate actor ids | Call `setActorIds(...)` | Actor ids are unique and stable in order rule |
| CON-003 | Property ids are deduplicated | Unit | Contract with duplicate property ids | Call `setPropertyIds(...)` | Property ids are unique and stable in order rule |
| CON-004 | Alias behavior remains consistent with entity aliases | Unit | Contract with alias list | Update aliases | Aliases are normalized and deduplicated |

#### Statement

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| STM-001 | Adding transaction preserves order | Unit | Empty statement with transaction ids | Add transaction ids in sequence | Order is preserved |
| STM-002 | Removing transaction updates membership only | Unit | Statement with multiple transaction ids | Remove one transaction | Remaining ids stay intact and ordered |
| STM-003 | `containsTransaction` reflects current membership | Unit | Statement with known transaction ids | Query membership | Returns correct boolean |

#### Transaction

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| TX-002 | Setting statement id updates relation state | Unit | Transaction without statement | Call `setStatementId(...)` | Relation state changes consistently |
| TX-003 | Setting property ids deduplicates values | Unit | Transaction with duplicate property ids | Call `setPropertyIds(...)` | Property ids are normalized and deduplicated |
| TX-004 | Finalization-ready transaction keeps domain invariants | Unit | Transaction from draft input | Call the finalization path used by application | Transaction remains valid for domain rules |

#### Analysis

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-001 | Analysis type and export format remain consistent | Unit | Analysis with type and export format | Update one side of the pair | Invalid combinations are rejected or normalized |
| ANL-002 | Config and filter parsing stay consistent | Unit | Analysis with config/filter input | Apply setters or construct through helper | Typed config/filter state is preserved |
| ANL-003 | Calculation adjustments flag is explicit and stable | Unit | Analysis with flag set | Read and mutate flag | Boolean state stays consistent |
| ANL-004 | Result/output snapshots remain internally consistent | Unit | Analysis with output fields | Update result fields | Output-related state stays coherent |

#### Annual

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-001 | Year validation rejects invalid values | Unit | Annual with invalid year | Construct or set year | Invalid year is rejected or normalized |
| ANN-002 | Analysis ids remain unique | Unit | Annual with duplicate analysis ids | Set analysis ids | Duplicates are removed deterministically |
| ANN-003 | Name mutation leaves year and analysis ids untouched | Unit | Annual with existing data | Rename annual | Only the name changes |

### 3. Values

#### Alias

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| VAL-AL-001 | Normalized alias value trims whitespace | Unit | Alias with surrounding whitespace | Normalize or construct | Stored alias is trimmed |
| VAL-AL-002 | Alias source and canonical value stay aligned | Unit | Alias with source/value pair | Update source or value | Pair remains internally consistent |

#### EntityName

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| VAL-NAME-001 | Valid names pass validation | Unit | Representative valid names | Call `isValid(...)` | Returns `true` |
| VAL-NAME-002 | Blank names fail validation | Unit | Empty or whitespace-only input | Call `isValid(...)` | Returns `false` |

#### MoneyAmount

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| VAL-MONEY-001 | Valid money values pass validation | Unit | Representative numeric values | Call `isValid(...)` | Returns `true` for valid amounts |
| VAL-MONEY-002 | Invalid money values fail validation | Unit | Non-finite or malformed values | Call `isValid(...)` | Returns `false` |

### 4. Policies

#### AliasPolicy

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| POL-AL-001 | Trim helper removes leading and trailing whitespace | Unit | Spaced string | Call trim helper | Returns canonical text |
| POL-AL-002 | Alias canonicalization is deterministic | Unit | Mixed alias inputs | Canonicalize aliases | Same input yields same normalized output |

#### TransactionPolicy

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| POL-TX-001 | Finalization check rejects incomplete draft data | Unit | Incomplete transaction draft input | Call finalization predicate | Returns `false` |
| POL-TX-002 | Finalization check accepts valid draft data | Unit | Valid transaction draft input | Call finalization predicate | Returns `true` |

#### AnalysisPolicy

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| POL-AN-001 | Analysis type/export format pair is valid | Unit | Representative type/format combinations | Call validation helper | Valid combinations pass |
| POL-AN-002 | Adjustment payload validation is deterministic | Unit | Representative adjustment data | Validate payload | Invalid shapes are rejected consistently |

#### AnnualPolicy

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| POL-ANN-001 | Analysis ids are unique | Unit | Analysis id list with duplicates | Validate uniqueness | Duplicates are rejected or removed as specified |
| POL-ANN-002 | Year validation mirrors the value object rule | Unit | Representative year values | Validate year | Same result as `Year::isValid(...)` |

#### StatementPolicy

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| POL-STM-001 | Transaction ids are trimmed and normalized | Unit | Statement policy helpers and spaced ids | Normalize a transaction id | The id is trimmed to its canonical form |
| POL-STM-002 | Transaction id lists are normalized deterministically | Unit | Mixed transaction id list | Normalize the list | Duplicate ids are removed and stable ordering is preserved |
| POL-STM-003 | Membership and index helpers stay deterministic | Unit | Known ordered transaction ids | Query membership and index helpers | Membership and position checks return stable results |

#### DraftMatchingPolicy

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| POL-DRAFT-001 | Known contract type extraction is stable | Unit | Catalog with named contracts | Ask for known types | Returns stable, deduplicated types |
| POL-DRAFT-002 | Actor and contract resolution is deterministic | Unit | Workspace catalog with aliases and names | Resolve text | Same text resolves to the same id or empty |

## Application

The application suite covers workspace orchestration, analysis execution,
draft finalization, snapshot projection, and port contracts.

### 1. Workspace boundary

#### WorkspaceFacade

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| WSF-001 | Snapshot projection reflects state changes | Unit | Facade with fresh storage manager | Create workspace and add/update/delete one actor | Snapshot callbacks publish the current workspace state consistently |
| WSF-002 | Catalog commands route through the facade boundary | Unit | Facade with writable workspace session | Send add and update actor commands | Actor is created, updated, and persisted through the boundary |
| WSF-003 | Workspace facade keeps annual derivation out of workspace reader port | Unit | Facade with annual, assigned analyses, and snapshot payloads | Query workspace snapshot through read port | Annual derivation is not exposed as a workspace reader method and remains an annual application concern |

#### WorkspaceCommandService

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| WCS-001 | Add actor command reaches catalog | Unit | Empty session state | Send add actor command | Actor appears in catalog and commit occurs once |
| WCS-002 | Update contract command rewrites relations | Unit | Existing contract in session state | Send update contract command | Relations update and commit occurs once |
| WCS-003 | Delete statement command removes owned transactions | Unit | Statement with linked transactions | Send delete statement command | Catalog state reflects ownership rule |
| WCS-004 | Add analysis command preserves typed parameters | Unit | Analysis command input | Send add analysis command | Typed fields are passed through consistently |
| WCS-005 | Insert transaction command preserves statement order and valuta | Unit | Statement with two transactions | Add a transaction with `insertAfterTransactionId` and `valuta` | The statement id order places the new transaction after the requested one and the transaction keeps its valuta |

#### WorkspaceQueryService

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| WQS-001 | Workspace snapshot reflects session state | Unit | Session with catalog and workflow data | Query snapshot | Snapshot matches current session state |
| WQS-002 | Contract types projection is sorted and deduplicated | Unit | Catalog with repeated contract types | Query types | Types are unique and ordered |
| WQS-003 | Load statement draft reconstructs transactions in order | Unit | Session with draft and transaction drafts | Load draft | Statement draft contains ordered transactions |
| WQS-004 | Import and export logs project as shared read state | Unit | Session with logs | Query logs | Returned collections match workflow state |

#### WorkspaceSnapshotProjector

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| WSP-001 | Session state projects to snapshot without loss of catalog data | Unit | Session state with all catalog collections | Project snapshot | Snapshot contains all expected catalog rows |
| WSP-002 | Workflow data projects to snapshot without loss of drafts and logs | Unit | Session state with drafts and logs | Project snapshot | Workflow data is preserved in the boundary model |
| WSP-003 | Empty session state projects to empty snapshot | Unit | Empty session state | Project snapshot | Snapshot is empty and stable |

#### WorkspaceStateManager

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| WSM-001 | Load round-trips a full session state | Integration | Repository bundle with catalog, drafts, and logs | Call `load()` and then project snapshot | Reconstructed state matches stored content |
| WSM-002 | Save preserves catalog and workflow separation | Integration | Session state with mixed content | Call `save(state)` | Persistence receives catalog and workflow data through the correct routes |
| WSM-003 | Strict validation rejects invalid state | Integration | Invalid session state | Enable strict validation and save | Save fails deterministically |

#### WorkspaceWorkflowService

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| WWS-001 | Saving a statement draft updates workflow state | Unit | Session with empty workflow state | Save one draft | Draft and its transaction drafts are stored together |
| WWS-002 | Clearing a draft removes dependent transactions | Unit | Session with saved draft and transaction drafts | Clear that draft | Draft and all related transaction drafts are removed |
| WWS-003 | Import logs replace workflow logs atomically | Unit | Session with existing import logs | Replace logs | Import log collection is replaced without residue |
| WWS-004 | Export logs replace workflow logs atomically | Unit | Session with existing export logs | Replace logs | Export log collection is replaced without residue |
| WWS-005 | Saving a statement draft avoids disk commit | Unit | Session with an opened workspace path | Save one draft | Draft state updates in memory while storage remains untouched until an explicit save/commit path |

### 2. Analysis application

#### AnalysisService

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANS-001 | Run analysis on a workspace catalog | Unit | Catalog with matching transactions | Call `runAnalysis(...)` | Result contains expected rows and aggregates |
| ANS-002 | Run analysis by id resolves analysis configuration | Unit | Workspace with analysis entity and catalog | Call `runAnalysisById(...)` | Analysis config is resolved and executed |
| ANS-003 | Filtered analysis only processes matching transactions | Unit | Workspace with filterable transactions | Apply restrictive request | Non-matching transactions are excluded |
| ANS-004 | Analysis output remains stable for identical input | Unit | Same workspace and request twice | Execute twice | Output is deterministic |

### 3. Annual result application

#### AnnualService

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANR-001 | Snapshot parser accepts canonical array payload form only | Unit | Analysis snapshots stored as `[]` with transaction objects | Build annual result for assigned analyses | Rows are projected from canonical array payloads and legacy wrapper/object formats are not part of the contract |
| ANR-002 | Row categorization is deterministic | Unit | Rows with exact duplicates, date-cluster amount variants, unique rows, and missing live matches | Build annual result | Rows are partitioned into `deduplicated`, `similar`, `divergent`, and `workspaceOnly` consistently |
| ANR-003 | Result statistics derive from categorized rows and live state | Unit | Mixed-year snapshots, missing-live rows, and status variants | Build annual result | `missingFromYear`, `mixedYear`, `duplicateCount`, `missingLive`, and status counters match projection rules |

### 4. Export application

#### ExportService

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| AEX-001 | Multi-format analysis export | Unit | Workspace snapshot with one table analysis and one plot analysis | Export `objectRequests` with analysis objects in formats `csv`, `xlsx`, `jpg`, and `png` into one target directory | Export result is `Ok`, all four output files are created, and format-specific ports are invoked successfully |
| AEX-002 | Export progress reports monotonic stage updates | Unit | Workspace snapshot with multiple analysis object requests and a progress callback | Execute export request and record callback values | Progress callback is invoked multiple times, values are monotonic, and reaches final completion stage |

### 5. Draft finalization

#### DraftFinalizer

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| DFT-001 | Finalizes a valid statement draft into catalog state | Unit | Catalog and valid draft | Call finalizer | Statement and transactions are created consistently |
| DFT-002 | Rejects invalid draft input | Unit | Catalog and invalid draft | Call finalizer | No catalog mutation occurs |
| DFT-003 | Preserves transaction ordering from draft positions | Unit | Draft with ordered transactions | Finalize draft | Final transactions follow draft order |
| DFT-004 | Applies references through domain rules, not duplicate logic | Unit | Draft with actor/property/contract text | Finalize draft | Finalization uses domain matching behavior |
| DFT-005 | Preserves draft valuta on finalized transactions | Unit | Valid transaction draft with a valuta value | Finalize draft | Final transaction keeps the draft valuta instead of clearing it |

#### DraftMatcher

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| DTM-001 | Actor resolution uses canonical alias matching | Unit | Catalog with actor aliases | Resolve actor text | Returns the expected actor id or empty |
| DTM-002 | Contract resolution uses canonical alias matching | Unit | Catalog with contract aliases | Resolve contract text | Returns the expected contract id or empty |
| DTM-003 | Derived draft state is stable | Unit | Catalog and draft input | Build derived state | Same input yields same derived state |

#### DraftMatcherProjection

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| DMP-001 | Actor rows are projected deterministically | Unit | Catalog with actors | Build projection | Rows reflect names and aliases |
| DMP-002 | Contract rows are projected deterministically | Unit | Catalog with contracts | Build projection | Rows reflect names, types, and relations |
| DMP-003 | Missing draft data yields empty projection safely | Unit | Empty catalog or draft | Build projection | Projection does not crash and returns empty state |
| DMP-004 | Contract top suggestion falls back to scored relation match | Unit | Catalog with actor/type-compatible contract and no direct contract candidate | Build projection | Derived state yields a stable contract top suggestion with confidence |
| DMP-005 | Contract allocatable mode overrides inferred allocatable fallback | Unit | Contract mode set to `non-allocatable` and historical allocatable transactions | Build projection | Effective allocatable is `false` unless manually overridden |

### 6. Port contracts

#### IWorkspaceReader

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| POR-R-001 | Reader exposes immutable workspace snapshot | Contract | Reader implementation | Call `workspaceSnapshot()` | Returned value is read-only boundary state |
| POR-R-002 | Reader exposes statement draft snapshot when present | Contract | Reader with stored draft | Call draft snapshot getter | Draft snapshot is returned or empty |
| POR-R-003 | Reader exposes current path in snapshot or metadata | Contract | Reader with open workspace | Query current path | Path is present and stable |

#### IWorkspaceWriter

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| POR-W-001 | Writer accepts lifecycle commands | Contract | Writer implementation | Call open/save/commit | Workspace lifecycle behaves as specified |
| POR-W-002 | Writer accepts command structs for catalog changes | Contract | Writer implementation | Send add/update/delete commands | Commands are applied through the boundary |
| POR-W-003 | Writer accepts workflow and log commands | Contract | Writer implementation | Send draft/log commands | Workflow state changes are routed correctly |
| POR-W-004 | Writer callback emits snapshot state | Contract | Writer with callback | Trigger state change | Callback receives `WorkspaceSnapshot` |

## Test File Guidance

Recommended file layout:

```text
core/
  tests/
    domain/
      catalog/TestWorkspaceCatalog.cpp
      entities/TestActor.cpp
      entities/TestProperty.cpp
      entities/TestContract.cpp
      entities/TestStatement.cpp
      entities/TestTransaction.cpp
      entities/TestAnalysis.cpp
      entities/TestAnnual.cpp
      policies/TestAliasPolicy.cpp
      policies/TestTransactionPolicy.cpp
      policies/TestAnalysisPolicy.cpp
      policies/TestAnnualPolicy.cpp
      policies/TestDraftMatchingPolicy.cpp
      values/TestAlias.cpp
      values/TestEntityName.cpp
      values/TestContractType.cpp
      values/TestAnalysisType.cpp
      values/TestExportFormat.cpp
      values/TestFilterSpec.cpp
      values/TestBookingDate.cpp
      values/TestMoneyAmount.cpp
      values/TestYear.cpp
    application/
      workspace/TestWorkspaceCommandService.cpp
      workspace/TestWorkspaceFacade.cpp
      workspace/TestWorkspaceQueryService.cpp
      workspace/TestWorkspaceSnapshotProjector.cpp
      workspace/TestWorkspaceStateManager.cpp
      workspace/TestWorkspaceWorkflowService.cpp
      analysis/TestAnalysisService.cpp
      annual/TestAnnualService.cpp
      export/TestExportService.cpp
      import/draft/TestDraftMatcher.cpp
      import/draft/TestDraftMatcherProjection.cpp
      import/draft/TestDraftFinalizer.cpp
    ports/
      workspace/TestWorkspaceReaderContract.cpp
      workspace/TestWorkspaceWriterContract.cpp
```

## Implementation Order

1. Domain entities and policies.
2. `WorkspaceCatalog` aggregate behavior.
3. Workspace command and workflow application services.
4. Snapshot projection and port contracts.
5. Analysis and annual result application behavior.

## Definition of Done

The core test suite is complete when:

- every domain entity has at least one focused behavior file
- every value object has a validation or normalization test
- every policy has a dedicated contract test file
- the workspace catalog aggregate is covered as an aggregate root
- the workspace application boundary is covered by snapshot, workflow, and port tests
- analysis has execution coverage through the application service and related domain rules
- the port contracts are locked with boundary-level tests
