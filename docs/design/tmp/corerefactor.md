# Core Refactor Roadmap

## Goal

Bring the `core` target as close as possible to a consistent 10/10 across the following quality areas:

1. Clean Code / Clean Architecture
2. SOLID
3. Domain Driven Design
4. Consistent Doxygen documentation
5. Dead code
6. Consistent formatting
7. Unnecessary, redundant, or consolidatable functions, classes, and files

This roadmap intentionally does **not** split `WorkspaceFacade` yet. We first reduce the public surface by removing convenience-only APIs and dead abstractions, then re-evaluate what remains.

## Working Rules

- Remove convenience-only functions instead of preserving parallel APIs.
- Prefer one canonical abstraction per concept.
- Keep behavior deterministic; avoid fallback-style compatibility paths.
- Make minimal, focused changes per step, but complete each cleanup area coherently.
- Do not change or add any tests, yet.
- Do not hide warnings; treat warnings as actionable signals.

## Current High-Level Findings

## Progress Snapshot

Completed so far:

- removed empty and orphaned core artifacts (`AnalysisFilter.*`, `RunAnalysisResult.h`, `AppStateFacade.*`, `AnalysisEngine.*`, `OutputTypeResolver.cpp`)
- removed `AnalysisService` empty PImpl
- removed convenience-only `WorkspaceFacade` overloads and remaining low-value helper methods
- consolidated `Analysis` onto canonical JSON-oriented naming (`configJson`, `exportStateJson`, `snapshotTransactionsJson`)
- consolidated alias collection access on `aliases()` by removing duplicate `aliasesForMatching()` entry points from core domain entities
- consolidated analysis output-type resolution into one canonical domain-policy path
- isolated technical hydration sections across entity headers and removed the duplicate `Transaction::allocatable()` accessor in favor of `isAllocatable()`
- reviewed the current `Analysis` / `Actor` / `WorkspaceCatalog` domain surface and deferred further API cuts where no low-risk domain win was justified
- normalized Doxygen in the touched analysis/workspace scope
- normalized formatting in the touched analysis/workspace implementation scope
- validated the current refactor state with `cmake --build --preset debug-app`

### Confirmed cleanup targets

- `core/include/core/application/workspace/WorkspaceFacade.h` contains a very broad API surface with likely convenience overloads beside command-based methods.
- `core/include/core/domain/entities/Analysis.h` and `core/src/domain/entities/Analysis.cpp` expose duplicate naming pairs such as `configuration` / `configJson`, `exportState` / `exportStateJson`, and `snapshotTransactions` / `snapshotTransactionsJson`.
- `core/src/application/analysis/AnalysisFilter.h` and `core/src/application/analysis/AnalysisFilter.cpp` exist as empty files while the active implementation lives in `core/src/application/analysis/internal/AnalysisFilter.h` and `.cpp`.
- `core/include/core/application/analysis/RunAnalysisResult.h` appears to be an orphaned alternative result type and is a dead-code candidate.
- `core/include/core/application/analysis/AnalysisService.h` and `core/src/application/analysis/AnalysisService.cpp` use an empty PImpl (`Impl`) with no current benefit.
- Doxygen coverage and depth are inconsistent across public and private core headers.
- Formatting and public API style are not yet fully consistent.

## Execution Strategy

Work from highest confidence / lowest risk toward broader refactors:

1. Remove dead files and obviously unused abstractions.
2. Remove convenience-only APIs and migrate call sites.
3. Consolidate duplicate domain concepts into one canonical API.
4. Consolidate duplicated business rules into one source of truth.
5. Standardize Doxygen and formatting.
6. Always delete rather than deprecate.
7. Reassess remaining architecture and DDD issues after the surface is smaller.

## Refactor Backlog

### Phase 1 - Dead code and orphan cleanup

#### 1.1 Remove empty analysis filter facade files
- [x] Confirm that `core/src/application/analysis/AnalysisFilter.h` and `.cpp` are not included anywhere.
- [x] Remove the empty files from the repository.
- [x] Ensure only the internal analysis filter implementation remains.

#### 1.2 Remove orphaned result abstractions
- [x] Confirm whether `core/include/core/application/analysis/RunAnalysisResult.h` has any references.
- [x] Remove it if unused.
- [ ] If partially used, migrate all users to the canonical `AnalysisResult` model and then remove it.

#### 1.3 Remove structural dead weight
- [x] Remove the empty `AnalysisService::Impl` PImpl if there is no ABI or compile-isolation reason to keep it.
- [x] Simplify the class to direct implementation.

### Phase 2 - Remove convenience-only APIs

#### 2.1 Audit `WorkspaceFacade` overloads
- [x] Enumerate all command-based methods vs. convenience overloads.
- [x] Classify each overload as either required integration API or pure convenience wrapper.
- [x] Remove all wrappers that only forward into command objects without adding domain value.

#### 2.2 Migrate call sites away from convenience overloads
- [x] Find all call sites of `WorkspaceFacade` convenience overloads.
- [x] Replace them with command-based calls.
- [x] Remove the obsolete overload declarations and definitions.

#### 2.3 Re-evaluate remaining `WorkspaceFacade` API surface
- [x] After convenience removal, re-check which methods are still truly needed.
- [x] Document what remains as intentional API surface.

### Phase 3 - Consolidate duplicated domain APIs

#### 3.1 Canonicalize `Analysis` configuration naming
- [x] Choose one canonical API for configuration payload access.
- [x] Remove or deprecate the duplicate naming pair `configuration` / `configJson`.
- [x] Migrate call sites to the canonical naming.

#### 3.2 Canonicalize export state naming
- [x] Choose one canonical API for export state access.
- [x] Remove or deprecate `exportState` / `exportStateJson` duplication.
- [x] Migrate call sites.

#### 3.3 Canonicalize snapshot transaction naming
- [x] Choose one canonical API for snapshot transactions access.
- [x] Remove or deprecate `snapshotTransactions` / `snapshotTransactionsJson` duplication.
- [x] Migrate call sites.

#### 3.4 Remove duplicate semantic entry points elsewhere in core
- [x] Check for similar paired APIs in other entities or application services.
- [x] Consolidate them to one language per concept.

### Phase 4 - Consolidate rules and strengthen domain boundaries

#### 4.1 Unify analysis output type resolution
- [x] Compare `Analysis::outputType()`, `AnalysisPolicy::resolveOutputType(...)`, and `AnalysisService` output resolution.
- [x] Move the rule to one canonical location.
- [x] Remove duplicated implementations.

#### 4.2 Reassess ownership of analysis configuration logic
- [x] Separate orchestration logic from pure domain rules where beneficial.
- [x] Keep domain decisions out of convenience/service glue.

#### 4.3 Tighten anemic domain areas where cheap wins exist
- [x] Review `Analysis`, `Actor`, and `WorkspaceCatalog` for domain invariants that can be expressed without broad redesign.
- [x] Prefer meaningful operations over raw setter-heavy usage where safe.

### Phase 5 - Doxygen consistency

#### 5.1 Standardize public header documentation
- [x] Ensure every public core header has `@file` and `@brief` in the active analysis/workspace cleanup scope.
- [x] Ensure public methods with non-trivial behavior have consistent `@param` / `@return` docs in the active analysis/workspace cleanup scope.

#### 5.2 Standardize internal header documentation
- [x] Add concise Doxygen to important internal headers that are part of core implementation structure in the active analysis scope.
- [x] Keep internal docs lighter than public API docs, but consistent.

#### 5.3 Fix inaccurate documentation details
- [x] Correct stale file path references in file headers in the touched scope.
- [x] Remove misleading or duplicated wording introduced by legacy aliases in the touched scope.

### Phase 6 - Formatting consistency

#### 6.1 Normalize touched files to the existing dominant style
- [x] Keep indentation, brace style, spacing, and include grouping consistent with surrounding code in the touched scope.
- [x] Reduce mixed single-line / multi-line style where it harms readability in the touched scope.

#### 6.2 Reduce oversized signatures where possible
- [ ] Prefer command/value objects over long primitive parameter lists.
- [ ] Only change signatures where the cleanup is already in scope.

### Phase 7 - Validation and scoring pass

#### 7.1 Build validation
- [x] Run repository builds using the configured CMake presets.
- [x] Fix all warnings or build errors caused by the refactor.


#### 7.2 Final score review
- [x] Re-review the `core` target against all seven quality categories.
- [x] Record remaining gaps explicitly if a true 10/10 is not yet reached.

### Final score review

| Area | Score | Notes |
| --- | --- | --- |
| Clean Code / Clean Architecture | 8.5/10 | The core surface is noticeably smaller and more coherent after removing dead files, convenience wrappers, and duplicated rule ownership. The main remaining gap is that persistence hydration still leaks into entity APIs through technical setters. |
| SOLID | 8/10 | Responsibility boundaries improved, especially around analysis rule ownership and convenience API removal. Remaining friction is concentrated around multi-layer mapping concerns and a few still-long primitive-heavy signatures. |
| Domain Driven Design | 8/10 | Canonical domain language improved (`configJson`, `exportStateJson`, `snapshotTransactionsJson`, `isAllocatable()`), and a few meaningful entity operations replaced raw orchestration. The main remaining compromise is technical persistence hydration living on the entities. |
| Consistent Doxygen documentation | 8/10 | The touched public and important internal headers are now materially more consistent. A true 10/10 would still require a broader sweep beyond the active cleanup scope. |
| Dead code | 9/10 | The obvious dead and orphaned artifacts found during this pass were removed successfully. The remaining risk is hidden low-value code outside the touched scope rather than anything currently confirmed. |
| Consistent formatting | 8/10 | The touched analysis/workspace/domain files now follow a more consistent local style. A full-repository 10/10 would require a wider normalization pass rather than selective cleanup. |
| Unnecessary / redundant / consolidatable code | 8.5/10 | Several real duplicates were removed and multiple concepts now have one canonical entry point. The most obvious remaining consolidation opportunity is the deferred DTO/mapper extraction and selected long signatures already noted in Phase 6. |

### Remaining gaps before a true 10/10

- technical hydration setters are still present on entities because removing them cleanly requires a broader DTO/mapper batch
- some signatures still pass several primitives where command/value objects would be cleaner, but that was intentionally kept out of the low-risk cleanup scope
- Doxygen and formatting are consistent in the touched scope, not yet guaranteed across the entire `core` target
- the final review found no obvious cheap duplicate comparable to the removed entity and analysis aliases, so the remaining gains are likely higher-cost architectural batches rather than more safe deletions

## Deferred follow-up - DTO and mapper extraction

This is intentionally deferred until the current low-risk entity cleanup settles.

### Why defer it now

- The current entity hydration setters are technical persistence seams, not convenience-only duplicates.
- Extracting DTOs and dedicated mappers would touch repositories, snapshot projection, import/export, and UI payload boundaries at once.
- The expected design win is real, but the current cost is multi-layer churn rather than a cheap cleanup.

### Recommended scope for a future batch

1. Introduce persistence-facing record/DTO types for the highest-churn entities first (`Transaction`, `Contract`, `Analysis`).
2. Move repository hydration away from entity technical setters into dedicated mapper code.
3. Keep entity construction focused on domain operations and validated state changes.
4. Re-check whether snapshot/projector payload code can reuse the same mapper layer without adding fallback paths.

### Expected payoff

- clearer separation between domain behavior and persistence hydration
- smaller technical surface on entities
- better alignment with DDD and clean architecture boundaries

### Expected cost and risk

- broad signature and mapping churn across persistence, application, and UI projection code
- temporary duplication during migration if not executed as one coherent batch
- higher regression risk than the current duplicate-removal work

## Recommended First Implementation Batch

Start with the highest-confidence batch that should improve quality immediately with limited behavioral risk:

- Remove empty `AnalysisFilter` wrapper files.
- Remove `RunAnalysisResult.h` if unused.
- Remove `AnalysisService` empty PImpl.
- Audit and remove `WorkspaceFacade` convenience-only overloads.
- Migrate all affected call sites.

## Done Criteria

This roadmap is complete when:

- convenience-only APIs are gone,
- dead files and unused abstractions are removed,
- duplicated domain naming is consolidated,
- analysis rules have one source of truth,
- Doxygen is consistent across the core surface,
- touched files follow one formatting style,
- builds pass,
- and a final review of the `core` target shows no obvious low-value redundancy left.
