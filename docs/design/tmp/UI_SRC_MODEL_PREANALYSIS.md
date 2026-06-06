# UI src/include model preanalysis

## Scope
This document only covers the UI C++ layer:
- `ui/include/ui/**`
- `ui/src/**`
- `ui/tests/unit/**`
- `ui/tests/interaction/**`

It intentionally does **not** expand into core, persistence, debug, or services yet.
The immediate goal is to understand whether the current UI model layer is still the right abstraction for the UI and how to structure future refactors and tests.

## Executive summary
The UI model layer is still relevant and not obsolete.
It acts as a bridge between the core domain models and QML/UI-facing list and selection abstractions.

Most UI list models are thin wrappers over core domain objects.
A smaller set of models are UI-specific adapters:
- `TransactionList`
- `ImportRunList`
- `ExportRunList`
- `TransactionDraftList`
- `TransactionFilter`
- `StatementDraft`

Those adapter models are likely where future refactors will matter most, because they carry UI-facing role names, cached projections, and convenience methods that are not purely core-domain logic.

## Current structure observed
### UI public headers
The UI public surface in `ui/include/ui/models` currently includes:
- `ActorList`
- `AnalysisList`
- `AnnualList`
- `ContractList`
- `ExportRunList`
- `ImportRunList`
- `PropertyList`
- `StatementList`
- `TransactionList`
- `TransactionDraftList`
- `TransactionFilter`
- `ImportSuggestion`
- `StatementDraft`

These headers are used by the UI state facade and QML-facing application context.

### UI implementation files
The matching implementations live in `ui/src/models`:
- `ActorList.cpp`
- `AnalysisList.cpp`
- `AnnualList.cpp`
- `ContractList.cpp`
- `ExportRunList.cpp`
- `ImportRunList.cpp`
- `PropertyList.cpp`
- `StatementList.cpp`
- `TransactionList.cpp`
- `TransactionDraftList.cpp`
- `TransactionFilter.cpp`
- `StatementDraft.cpp`

## Direct mapping to core models
The following UI list models are mostly direct projections of core domain objects:
- `ActorList` → `core::domain::Actor`
- `PropertyList` → `core::domain::Property`
- `ContractList` → `core::domain::Contract`
- `StatementList` → `core::domain::Statement`
- `AnnualList` → `core::domain::Annual`
- `AnalysisList` → `core::domain::Analysis`

These models mostly do three things:
1. expose QML-compatible role names,
2. translate core fields to QVariant data,
3. provide add/remove/update helpers needed by UI workflows.

### What this means
These are generally still appropriate.
They do not look like redundant copies of core models.
They are UI-facing list adapters over core objects.

The main question is not whether they should exist.
The main question is whether their role sets are still minimal and aligned with the actual UI.

## UI-specific adapter models
### TransactionList
`TransactionList` is more than a direct projection.
It includes:
- role names for QML
- cached contract type lookup
- transaction-to-payload mapping
- row replacement and identity index rebuilding
- a `get(int)` convenience API

This suggests the model is a hybrid of:
- a QML-facing list model,
- a payload projection,
- and a small adapter over core transaction state.

It is likely still useful, but it is also a strong candidate for future simplification if some roles are no longer needed by the UI.

### ImportRunList and ExportRunList
These are UI-specific adapter list models for run-log display.
They are not direct core-model mirrors.
They exist because the UI needs a flattened, view-friendly representation of import/export activity.

These should probably stay, but their payload shape should be reviewed against actual usage in QML and interaction tests.
If a role is never used in the UI, it may be a candidate for removal.

### TransactionDraftList and StatementDraft
These are UI workflow models, not core domain mirrors.
They support draft editing, navigation, and workflow-specific state.
They are likely still needed for interaction tests and draft-related QML views.

### TransactionFilter
This is a helper model/state object for UI filtering, not a domain entity.
It should be evaluated primarily for testability and whether its fields are still the minimum required for the current UI.

## Does the UI still use these models?
Yes.
The UI uses these models heavily.
The clearest consumers are:
- QML views and panels that bind to `StateFacade` collections,
- sidebars and selection panels,
- import/export views,
- analysis/booking/property/contract forms,
- and UI interaction tests that work through the facade and model role names.

### Most visible consumers
From the current codebase, these UI models are used in:
- actor/property/contract sidebars and forms
- booking statement and transaction views
- analysis views and filters
- annual views and panels
- import/export views
- run log lists in the common/shared components

So the short answer is: **yes, they are used, and likely still needed**.
The bigger question is whether each model exposes too many fields or roles.

## Initial assessment of model freshness
### Likely still reasonable
- `ActorList`
- `PropertyList`
- `ContractList`
- `StatementList`
- `AnnualList`
- `AnalysisList`

These look like standard UI-facing model adapters with fields that match the current UI workflows.

### Likely worth reviewing carefully
- `TransactionList`
- `TransactionDraftList`
- `TransactionFilter`
- `ImportRunList`
- `ExportRunList`
- `StatementDraft`

These are more workflow-heavy and more likely to accumulate fields that are no longer needed.
They are also the most likely to benefit from refactor-first cleanup before more tests are added.

## Core-to-UI mapping observations
### Core model coverage is broad
The core domain already has the expected entities:
- `Actor`
- `Property`
- `Contract`
- `Statement`
- `Transaction`
- `TransactionDraft`
- `Analysis`
- `Annual`
- `AppState`
- `ImportLog`
- `ExportLog`

### UI models do not appear to duplicate all core fields blindly
The UI list models usually expose only what the UI needs.
However, some models expose fields that may now be historical leftovers or convenience roles.
That is especially likely in `TransactionList` and the run-log models.

### The likely architectural pattern
The UI layer appears to use three patterns:
1. **Direct list adapters** over core entities.
2. **Workflow adapters** for draft / run / filter state.
3. **Facade/projection state** that flattens core state for QML.

That is a reasonable pattern and does not need a full redesign.
What it needs next is a targeted review of whether the adapters are still minimal and whether the state facade should be the place that owns certain projections.

## What should be checked next
### For UI list models
For each model, check:
- which QML views consume it,
- which roles are actually referenced,
- whether any role exists only for old implementation reasons,
- whether the model shape still mirrors the needed UI, or if a slimmer projection would do.

### For selection/state models
Check whether these still need their current breadth:
- `SessionStore`
- `SessionSelection`
- `SelectionState`
- `StateFacade`
- `StateFacadeProjection`
- `SessionModels`
- `SessionModelProjection`
- `SessionMutationState`

These are the likely seams for reducing UI coupling.

### For interaction and unit tests
Current UI C++ tests are only a small set:
- `ui/tests/unit/TestAnalysisPayloadMapper.cpp`
- `ui/tests/unit/TestImportSuggestionService.cpp`
- `ui/tests/interaction/TestImportState.cpp`

That means the UI C++ layer is under-covered compared to the QML layer.
There is likely room to add meaningful tests once the state/model seams are clearer.

## Preliminary answer to the model-size question
Your instinct is plausible:
some UI models may currently expose more fields than the UI actually needs.

But the first step is not to remove fields immediately.
The first step is to map each role and helper to the actual UI consumer.
If a role is not used by any view, panel, or interaction test, it becomes a likely candidate for removal or relocation.

`TransactionList` is the most suspicious model in this respect because it already mixes direct transaction data with payload-style projection logic.

## Recommended structure for the next phase
For the next UI C++ phase, do it in this order:
1. inventory `ui/src/models` and `ui/include/ui/models` role-by-role,
2. map every role to its QML or C++ consumer,
3. identify roles that are unused or duplicated,
4. split pure mapping logic from orchestration if needed,
5. then add or adjust unit/interaction tests around the surviving seams.

## Test-structure recommendation for UI
Do **not** mirror every production file 1:1 in tests.
Instead, group tests by domain and behavior:
- `ui/tests/unit/models/`
- `ui/tests/unit/state/`
- `ui/tests/unit/import/`
- `ui/tests/unit/export/`
- `ui/tests/interaction/import/`
- `ui/tests/interaction/export/`
- `ui/tests/interaction/state/`
- `ui/tests/interaction/window/`

That keeps the test tree readable without becoming a carbon copy of `ui/src`.

## Handoff prompt for the next step
Use this prompt next time if you want to continue the UI C++ pass:

> Stay only in `ui/include` and `ui/src` for now. Map every `ui/include/ui/models/*` and `ui/src/models/*` type to its real consumer in QML or C++ and decide which models are direct adapters, which are workflow adapters, and which are state/projection helpers. Then inspect `StateFacade`, `SelectionState`, `SessionStore`, `SessionModels`, and `StateFacadeProjection` for overcoupling. Do not remove fields or roles until you can show whether they are consumed by a sidebar, form, panel, or interaction test. The goal is to decide whether the UI model layer should stay as-is, be slimmed down, or have logic moved into clearer projection helpers. Keep tests grouped by behavior and domain, not as a 1:1 copy of the source tree. After that, propose the next refactor batch and the exact test additions or updates it needs.

## Final note
The current UI model layer does look usable and still relevant.
It is not obviously obsolete.
The main question is **not** whether the UI models exist.
The main question is whether some of them should be slimmer and whether projection logic should be moved into clearer helpers before more tests are added.
