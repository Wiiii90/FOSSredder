# Core Domain Refactor 2 - Status

## Current migration progress

### Completed architecture shifts
- Introduced domain catalog aggregate split (`WorkspaceCatalog`) and application session split (`WorkspaceSessionState`).
- Introduced workspace ports and boundary contracts (`IWorkspaceReader`/`IWorkspaceWriter`, snapshot/command models).
- Migrated large parts of application orchestration to new workspace services (`WorkspaceCommandService`, `WorkspaceWorkflowService`, projector/query paths).
- Migrated storage/session state flow to `WorkspaceSessionState` (storage manager, state manager, persistence adapter entry points).

### Compile blocker waves already resolved
- `WorkspaceState` alias/redefinition conflict in analysis/export paths was removed from active blocker set.
- Incomplete-type ownership issues fixed by moving service destructors out-of-line:
  - `WorkspaceWorkflowService`
  - `WorkspaceCommandService`
- Analysis/export path significantly stabilized by moving hot paths to `WorkspaceCatalog` signatures.
- Large UI surface migrated from legacy public field access to getter/setter API:
  - models: `IndexedListModel`, `TransactionList`, `AnalysisList`, `AnnualList`, `ActorList`, `PropertyList`, `StatementList`, `ContractList`
  - state: `SessionModelProjection`, `SessionMutationState`, `SelectionState`, `StateFacadeProjection`
  - controllers: `TransactionController`, `StatementController`, `PropertyController`, `ActorController`, `ContractController`
  - payload mapping: `EntityPayloadMapper`

## Latest verified build state
- Preset configure works: `cmake --preset app`.
- Serial preset full build now succeeds end-to-end:
  - `cmake --build --preset debug-app -- /m:1 /p:UseMultiToolTask=false /p:CL_MPCount=1`
- App link stage completes and produces:
  - `.build/app/bin/Debug/fossredder.exe`

## Latest completed migration wave (this batch)
- Fixed UI/app integration follow-up in `app/src/main_qml.cpp`:
  - removed duplicate snapshot declaration,
  - migrated import snapshot provider to `WorkspaceCatalog` setter/getter API.
- Continued large persistence migration to entity getter/setter APIs:
  - `SqliteTransactionRepository.cpp`
  - `SqliteStatementRepository.cpp`
  - `SqlitePropertyRepository.cpp`
  - `SqliteContractRepository.cpp`
  - `SqliteAnnualRepository.cpp`
  - `SqliteAnalysisRepository.cpp`
  - `SqliteActorRepository.cpp`
  - `WorkspaceStateStore.cpp`
- Fixed infra constant usage:
  - `infra/analysis-image-renderer/src/OpenCvAnalysisImageRendererAdapter.cpp`
  - `kTypeCalc` -> `kTypeCalculation`
- Resolved linker blockers by restoring missing core compilation units in `core/CMakeLists.txt`:
  - `src/application/workspace/WorkspaceSnapshotProjector.cpp`
  - `src/application/workspace/WorkspaceWorkflowService.cpp`
  - `src/application/workspace/WorkspaceCommandService.cpp`
  - `src/domain/catalog/WorkspaceCatalog.cpp`
  - `src/application/analysis/internal/AnalysisFilter.cpp`
- Implemented missing policy symbol:
  - `core::domain::policies::annual::normalizeIds(...)` in `AnnualPolicy.cpp`
- Fixed remaining compile mismatches:
  - enum->int conversion in `WorkspaceSnapshotProjector.cpp`
  - include path and operator token type in `AnalysisFilter.cpp`

## What this means
- Refactor reached a **stable buildable baseline** again after the large API migration waves.
- Core/domain/application split and getter/setter migration now compile through full app target.
- Next work can focus on cleanup/validation instead of blocker-driven compile rescue.

## Immediate next batch
1. Run targeted tests (starting with changed core/persistence areas).
2. Scan for remaining legacy field access patterns and clean residual hotspots.
3. Keep this status file updated per batch with test/build outcomes.

## Latest cleanup/test batch
- Executed test preset:
  - `ctest --preset debug-tests --output-on-failure`
  - Result: no discovered tests in the current debug-tests run context.
- Ran source scan for residual legacy member-access patterns in `core/ui/persistence/app/infra`.
  - Remaining direct field access hotspots are primarily in workflow draft/log models and selected unit tests.
- Cleaned one larger residual hotspot in:
  - `persistence/tests/unit/TestWorkspaceStateStore.cpp`
  - Migrated test setup/assertions from legacy direct entity/catalog fields to current getter/setter + catalog accessors.
- Revalidated with full serial app preset build:
  - `cmake --build --preset debug-app -- /m:1 /p:UseMultiToolTask=false /p:CL_MPCount=1`
  - Result: successful end-to-end build; `fossredder.exe` produced.

## Latest continuation batch
- Performed additional non-test cleanup scan for residual catalog/entity legacy access patterns.
- Verified remaining hits in `core/ui/persistence/app/infra` are valid accessor-based usages or workflow draft/log models that still intentionally expose plain data fields.
- No further production-file migration edits were required in this pass.
- Build baseline remains green.

## Latest refactor continuation (AliasValue consolidation)
- Continued core-domain cleanup in line with `docs/design/coredomainrefactor2.md` alias consolidation target.
- Removed `AliasValue` from entity APIs:
  - `core/include/core/domain/entities/Actor.h`
  - `core/include/core/domain/entities/Property.h`
  - `core/include/core/domain/entities/Contract.h`
- Updated entity implementations to string-based alias mutation flow:
  - `core/src/domain/entities/Actor.cpp`
  - `core/src/domain/entities/Property.cpp`
  - `core/src/domain/entities/Contract.cpp`
- Removed obsolete AliasValue files:
  - `core/include/core/domain/values/AliasValue.h`
  - `core/src/domain/values/AliasValue.cpp`
- Updated core build definition:
  - removed `src/domain/values/AliasValue.cpp` from `core/CMakeLists.txt`
- Validation:
  - full workspace build passed (`Buildvorgang erfolgreich`).

## Latest workspace bridge-pruning batch
- Focus aligned to clarified user scope: remove bridge/compatibility files in `core/application/workspace` to match target structure from `docs/design/coredomainrefactor2.md`.
- Folded catalog mutation stack into `WorkspaceCommandService`:
  - integrated former `WorkspaceCatalogService`, `WorkspaceMutationService`, `CatalogDraftAppliers.h`, and `CatalogMutationHelpers.h` behavior directly in `core/src/application/workspace/WorkspaceCommandService.cpp`.
  - moved command-side input structs into `core/include/core/application/workspace/WorkspaceCommandService.h`.
- Folded workflow draft/log mutation stack into `WorkspaceWorkflowService`:
  - integrated former `WorkspaceDraftMutationService` and `WorkspaceLogMutationService` behavior directly in `core/src/application/workspace/WorkspaceWorkflowService.cpp`.
- Removed obsolete workspace bridge files:
  - `core/include/core/application/workspace/WorkspaceCatalogService.h`
  - `core/src/application/workspace/WorkspaceCatalogService.cpp`
  - `core/include/core/application/workspace/WorkspaceMutationService.h`
  - `core/src/application/workspace/WorkspaceMutationService.cpp`
  - `core/include/core/application/workspace/WorkspaceDraftMutationService.h`
  - `core/src/application/workspace/WorkspaceDraftMutationService.cpp`
  - `core/include/core/application/workspace/WorkspaceLogMutationService.h`
  - `core/src/application/workspace/WorkspaceLogMutationService.cpp`
  - `core/src/application/workspace/CatalogDraftAppliers.h`
  - `core/src/application/workspace/CatalogMutationHelpers.h`
- Updated build wiring in `core/CMakeLists.txt` to remove deleted workspace bridge headers/sources.
- Validation pending in next step: full preset build (`cmake --build --preset debug-app -- /m:1 /p:UseMultiToolTask=false /p:CL_MPCount=1`).

## Latest continuation batch (UI port migration, part 1)
- Continued workspace cleanup by reducing dependence on legacy application workspace interfaces in UI controllers.
- Migrated these controllers from `core::application::IWorkspaceReader/IWorkspaceWriter` to `core::ports::workspace::IWorkspaceReader/IWorkspaceWriter`:
  - `ui/include/ui/controllers/ActorController.h`
  - `ui/include/ui/controllers/PropertyController.h`
  - `ui/include/ui/controllers/ContractController.h`
  - `ui/include/ui/controllers/AnnualController.h`
  - `ui/include/ui/controllers/StatementController.h`
  - `ui/include/ui/controllers/TransactionController.h`
- Migrated corresponding controller implementations to typed workspace commands and snapshot-based reads:
  - `ui/src/controllers/ActorController.cpp`
  - `ui/src/controllers/PropertyController.cpp`
  - `ui/src/controllers/ContractController.cpp`
  - `ui/src/controllers/AnnualController.cpp`
  - `ui/src/controllers/StatementController.cpp`
  - `ui/src/controllers/TransactionController.cpp`
- Replaced primitive mutation calls with `WorkspaceCommands` payloads for actor/property/contract/annual/statement/transaction operations.
- Replaced direct `catalogState()` read usage in migrated controllers with `workspaceSnapshot()` mapping.
- Remaining legacy UI dependency surface is now concentrated in:
  - `AnalysisController`
  - `DraftController`
  - `StorageController`
  (these still need read-path projection for `catalogState`/`presentWorkspace`/`contractTypes`-style behaviors before interface deletion).
- Validation:
  - workspace build successful (`Buildvorgang erfolgreich`).

## Latest continuation batch (UI port migration, part 2)
- Continued migration of remaining UI workspace dependencies to port interfaces.
- Migrated `StorageController` from legacy application workspace interfaces to workspace ports:
  - `ui/include/ui/controllers/StorageController.h`
  - `ui/src/controllers/StorageController.cpp`
  - replaced `presentWorkspace` usage with `workspaceSnapshot`-derived `WorkspacePresentation`.
- Migrated `AnalysisController` to workspace ports and typed commands:
  - `ui/include/ui/controllers/AnalysisController.h`
  - `ui/src/controllers/AnalysisController.cpp`
  - replaced primitive analysis mutation calls with `core::ports::workspace::AnalysisCommand`.
  - replaced legacy `catalogState` and `contractTypes()` reader path with snapshot-based projection.
- Migrated `DraftController` to workspace ports and typed workflow commands:
  - `ui/include/ui/controllers/DraftController.h`
  - `ui/src/controllers/DraftController.cpp`
  - replaced primitive add/finalize/save draft paths with typed workspace commands.
  - added local snapshot-to-domain conversion helper to rebuild `WorkspaceState` for import matcher service workflows.
- Cleaned guard utility dependencies:
  - `ui/include/ui/util/CoreFacadeGuard.h` no longer includes legacy workspace interface headers.
- Resulting UI/controller scope no longer references:
  - `core/application/workspace/IWorkspaceReader.h`
  - `core/application/workspace/IWorkspaceWriter.h`
- Validation:
  - full workspace build successful (`Buildvorgang erfolgreich`).

## Latest continuation batch (final WorkspaceState compatibility deletion)
- Completed the requested full compatibility cleanup for legacy workspace aliases.
- Deleted the remaining compatibility files:
  - `core/include/core/application/workspace/Workspace.h`
  - `core/include/core/domain/WorkspaceState.h`
  - `core/src/domain/WorkspaceState.cpp`
- Migrated remaining `WorkspaceState` usages and includes to direct `WorkspaceCatalog` usage across core/ui/app/persistence.
  - Updated workspace services/session/catalog mutation paths (`WorkspaceSession`, `WorkspaceCommandService`, `WorkspaceFacade`, `WorkspaceStateManager`, import matcher stack, policy helpers).
  - Updated UI controllers/state/export/import draft paths to `core::domain::catalog::WorkspaceCatalog` signatures.
  - Updated app bootstrap adapters/providers in `main_qml.cpp` and includes in `main.cpp`.
  - Updated precompiled headers (`core/include/core/pch.h`, `ui/include/ui/pch.h`, `persistence/include/persistence/pch.h`).
  - Updated CMake source/header lists to remove deleted compatibility files (`core/CMakeLists.txt`).
- Validation:
  - full preset build successful: `cmake --build --preset debug-app`.

## Latest continuation batch (legacy workspace interfaces removed)
- Removed final legacy application workspace interface dependency surface.
- Updated `WorkspaceFacade` to inherit directly from workspace ports:
  - `core/include/core/application/workspace/WorkspaceFacade.h`
  - now implements `core::ports::workspace::IWorkspaceReader` + `core::ports::workspace::IWorkspaceWriter` directly.
  - retained compatibility helper methods as non-port methods (no legacy interface inheritance required).
- Deleted obsolete legacy interface headers:
  - `core/include/core/application/workspace/IWorkspaceReader.h`
  - `core/include/core/application/workspace/IWorkspaceWriter.h`
- Updated build list:
  - removed deleted headers from `core/CMakeLists.txt` (`CORE_PUBLIC_HEADERS`).
- Validation:
  - full workspace build successful (`Buildvorgang erfolgreich`).
