# Core Domain

This document consolidates the former `DOMAIN_AREA_PLAN.md` and
`DOMAIN_REFACTOR_PLAN.md` into a single catalog for the current `core/domain`
model.

The focus is deliberately practical:
- document the current domain objects as they exist today
- separate entities, value objects, and policies cleanly
- make the current invariants explicit
- keep a compact, concrete refactor backlog at the end

## Scope

`core/domain` is the part of the codebase that should contain:
- domain entities with behavior and invariants
- value objects for normalized business concepts
- reusable domain policies for shared rules and comparisons

It should not become:
- a dumping ground for workflow DTOs
- an application-state bag for import/export audit data
- a place for UI convenience models

The current codebase is already past the "anemic model" stage, but it still
contains some structural compromises, especially around `WorkspaceState` and
the amount of public mutable state inside entities.

## Domain Overview

The currently relevant domain concepts are:
- `Actor`
- `Property`
- `Contract`
- `Statement`
- `Transaction`
- `Analysis`
- `Annual`
- `WorkspaceState`

The main value objects are:
- `Alias`
- `AliasUsage`
- `AliasValue`
- `EntityName`
- `ContractType`
- `AnalysisType`
- `ExportFormat`
- `FilterSpec`
- `BookingDate`
- `MoneyAmount`
- `Year`

The current shared policies are:
- `AliasPolicy`
- `AnalysisPolicy`
- `AnnualPolicy`
- `TransactionPolicy`
- `DraftMatchingPolicy`

---

## Entities

### `Actor`

File: `core/include/core/domain/entities/Actor.h`

`Actor` represents a named catalog entry that can be matched via aliases.
It is one of the simplest business entities in the current model, but it is
still a real domain concept, not just a UI row.

#### State

Current public state:
- `id`
- `name`
- `aliases`
- `aliasUsage`
- `createdAt`
- `updatedAt`

#### Behavior

Implemented methods:
- `rename(EntityName)`
- `rename(std::string)`
- `setAliases(std::vector<Alias>)`
- `addAlias(Alias)`
- `removeAlias(AliasValue)`
- `removeAlias(std::string)`
- `clearAliases()`
- `recordAliasHit(AliasValue, std::string timestamp = {})`
- `aliasesForMatching() const`
- `hasAlias(const std::string&) const`
- `aliasCount() const`
- `hasAliasUsage(const std::string&) const`
- `clearAliasUsage()`
- `containsAlias(const std::string&) const`

#### Invariants

Currently enforced or implied:
- aliases are normalized through `AliasPolicy`
- duplicate alias values are rejected on insertion
- empty alias values are ignored
- alias usage is removed together with the alias

Still not enforced as hard invariants:
- `name` can still be written directly
- `id` is still plain string state
- workspace-level uniqueness is handled outside the entity

#### Notes

`Actor` is already reasonably domain-shaped. The biggest remaining improvement
would be stronger encapsulation, so callers cannot bypass normalization by
writing public fields directly.

### `Property`

File: `core/include/core/domain/entities/Property.h`

`Property` mirrors `Actor` structurally. It is a separate concept because the
matching and relationship semantics are different, even if the alias handling
is shared.

#### State

Current public state:
- `id`
- `name`
- `aliases`
- `aliasUsage`
- `createdAt`
- `updatedAt`

#### Behavior

Implemented methods:
- `rename(EntityName)`
- `rename(std::string)`
- `setAliases(std::vector<Alias>)`
- `addAlias(Alias)`
- `removeAlias(AliasValue)`
- `removeAlias(std::string)`
- `clearAliases()`
- `recordAliasHit(AliasValue, std::string timestamp = {})`
- `aliasesForMatching() const`
- `hasAlias(const std::string&) const`
- `aliasCount() const`
- `hasAliasUsage(const std::string&) const`
- `clearAliasUsage()`
- `containsAlias(const std::string&) const`

#### Invariants

Same alias rules as `Actor`:
- alias normalization is shared
- duplicate alias values are rejected
- empty aliases are removed during normalization
- alias usage is kept in sync when an alias is removed

#### Notes

`Property` is in good shape conceptually. Its main remaining issue is the same
as `Actor`: the model is rich enough, but the state is still easy to mutate
outside the methods.

### `Contract`

File: `core/include/core/domain/entities/Contract.h`

`Contract` is a richer catalog entity. It combines name, type, aliases, and
relations to actors and properties.

#### State

Current public state:
- `id`
- `name`
- `type`
- `actorIds`
- `propertyIds`
- `aliases`
- `aliasUsage`
- `createdAt`
- `updatedAt`

#### Behavior

Implemented methods:
- `rename(EntityName)`
- `rename(std::string)`
- `setType(ContractType)`
- `setType(std::string)`
- `setActorIds(std::vector<std::string>)`
- `assignActor(std::string)`
- `addActorId(std::string)`
- `removeActor(std::string)`
- `removeActorId(const std::string&)`
- `clearActorIds()`
- `setPropertyIds(std::vector<std::string>)`
- `addPropertyId(std::string)`
- `removeProperty(std::string)`
- `removePropertyId(const std::string&)`
- `clearPropertyIds()`
- `setAliases(std::vector<Alias>)`
- `addAlias(Alias)`
- `removeAlias(AliasValue)`
- `recordAliasHit(AliasValue, std::string timestamp = {})`
- `hasName() const`
- `aliasesForMatching() const`
- `hasAlias(const std::string&) const`
- `aliasCount() const`
- `hasAliases() const`
- `hasAliasUsage() const`
- `clearAliases()`
- `containsActorId(const std::string&) const`
- `containsPropertyId(const std::string&) const`
- `containsActor(const std::string&) const`
- `containsProperty(const std::string&) const`
- `hasSingleActor() const`
- `hasActorRelations() const`
- `hasPropertyRelations() const`
- `hasRelations() const`
- `relationCount() const`
- `actorCount() const`
- `propertyCount() const`
- `hasType() const`
- `isConfigured() const`
- `isMatchingReady() const`
- `isStandalone() const`

#### Invariants

Implemented today:
- actor and property lists are normalized to trimmed, unique ids
- alias behavior is shared through `AliasPolicy`
- `assignActor` resets existing actor relations before storing a single id
- `addActorId` and `addPropertyId` avoid duplicates

Design notes from the plans:
- the schema currently still supports many-to-many relations for both actors
  and properties
- the product rule "exactly one actor per contract" is not enforced
- if that rule is ever adopted, schema and domain need to change together

#### Notes

`Contract` is one of the most expressive entities in the model, but it is also
one of the places where business policy and persistence shape still overlap.
That is not wrong, but the domain should remain explicit about whether the
relationship lists are actual invariants or just catalog metadata.

### `Statement`

File: `core/include/core/domain/entities/Statement.h`

`Statement` is an ordered container of transaction ids. The order is part of
the domain meaning.

#### State

Current public state:
- `id`
- `name`
- `transactionIds`
- `createdAt`
- `updatedAt`

#### Behavior

Implemented methods:
- `rename(EntityName)`
- `rename(std::string)`
- `addTransaction(std::string)`
- `assignTransaction(std::string)`
- `setTransactionIds(std::vector<std::string>)`
- `insertTransaction(std::string, std::size_t position)`
- `removeTransaction(const std::string&)`
- `clearTransaction(std::string)`
- `indexOfTransaction(const std::string&) const`
- `hasTransactionAt(std::size_t) const`
- `moveTransaction(const std::string&, std::size_t newPosition)`
- `containsTransaction(const std::string&) const`
- `contains(std::string) const`
- `clearTransactions()`
- `transactionCount() const`
- `empty() const`
- `hasTransactions() const`

#### Invariants

Implemented today:
- transaction ids are trimmed before storage
- duplicate ids are rejected
- inserting an existing id first removes the old entry
- ordering is stable and preserved

#### Notes

`Statement` is already close to a useful aggregate-like structure.
What is still missing is a dedicated policy or clearer ownership semantics,
especially around the relationship between `Statement.transactionIds` and
`Transaction.statementId`.

### `Transaction`

File: `core/include/core/domain/entities/Transaction.h`

`Transaction` is a core domain object and one of the main candidates for
future rule tightening.

#### State

Current public state:
- `id`
- `name`
- `bookingDate`
- `valuta`
- `amount`
- `status`
- `contractId`
- `actorId`
- `statementId`
- `allocatable`
- `propertyIds`
- `createdAt`
- `updatedAt`

#### Behavior

Implemented methods:
- constructor with name, booking date, valuta, amount, allocatable
- `setName(std::string)`
- `setBookingDate(BookingDate)`
- `setBookingDate(std::string)`
- `setValuta(std::string)`
- `setAmount(MoneyAmount)`
- `setAmount(double)`
- `setStatus(Status)`
- `setContractId(std::string)`
- `setActorId(std::string)`
- `setStatementId(std::string)`
- `setAllocatable(bool)`
- `assignContract(std::string)`
- `clearContract()`
- `assignActor(std::string)`
- `clearActor()`
- `assignToStatement(std::string)`
- `clearStatement()`
- `clearProperties()`
- `hasStatus(Status) const`
- `hasStatement() const`
- `hasContract() const`
- `hasActor() const`
- `hasProperties() const`
- `propertyCount() const`
- `belongsToStatement(const std::string&) const`
- `isAssigned() const`
- `isAllocatable() const`
- `markUnverified()`
- `markVerified()`
- `markCompleted()`
- `hasRelations() const`
- `isStandalone() const`
- `setPropertyIds(std::vector<std::string>)`
- `addPropertyId(std::string)`
- `removePropertyId(const std::string&)`
- `containsPropertyId(const std::string&) const`
- `applyDraft(...)`
- `finalizeFromDraft()`
- `clearDraftAssignments()`

#### Invariants

Currently enforced:
- booking dates are normalized through `BookingDate`
- amounts are normalized through `MoneyAmount`
- property ids are trimmed and deduplicated through transaction policy
- status transitions only move forward
- `finalizeFromDraft()` only clears draft-related fields when the policy check
  passes

Important current behavior:
- `finalizeFromDraft()` still implicitly encodes a workflow rule rather than a
  pure entity rule

#### Notes

This is the entity with the clearest path for additional domain behavior.
The current implementation already contains several rules, but the model still
mixes core transaction state with import-finalization behavior. That is fine for
now, but it is the place where further refinement would pay off fastest.

### `Analysis`

File: `core/include/core/domain/entities/Analysis.h`

`Analysis` is a product entity that stores analysis configuration and output
state. It has the most room for future domain tightening.

#### State

Current public state:
- `id`
- `name`
- `type`
- `configJson`
- `filterSpec`
- `exportFormat`
- `includeCalcAdjustments`
- `exportStateJson`
- `snapshotTransactionsJson`
- `adjustments`
- `createdAt`
- `updatedAt`

#### Behavior

Implemented methods:
- `rename(EntityName)`
- `rename(std::string)`
- `setType(AnalysisType)`
- `setType(std::string)`
- `setConfigJson(std::string)`
- `setFilterSpec(FilterSpec)`
- `setFilterSpec(std::string)`
- `setExportFormat(ExportFormat)`
- `setExportFormat(std::string)`
- `enableCalcAdjustments(bool)`
- `setIncludeCalcAdjustments(bool)`
- `setExportStateJson(std::string)`
- `setSnapshotTransactionsJson(std::string)`
- `clearAdjustments()`
- `setAdjustment(std::string, double)`
- `removeAdjustment(const std::string&)`
- `hasAdjustment(const std::string&) const`
- `adjustmentCount() const`
- `hasType() const`
- `hasExportFormat() const`
- `hasConfig() const`
- `hasFilterSpec() const`
- `hasSnapshotTransactions() const`
- `isReadyForExport() const`
- `isConfigured() const`
- `isTabular() const`
- `isChartLike() const`
- `supportsExportFormat(const std::string&) const`
- `hasAdjustments() const`
- `isResultReady() const`
- `applyDraft(...)`
- `setConfigPayload(std::string)`
- `hasFilterConfiguration() const`
- `outputType() const`

#### Invariants

Currently enforced:
- type, export format, and filter spec are normalized through value objects
- `adjustments` is a real model element, not just a raw blob
- readiness checks delegate to `AnalysisPolicy`

Still intentionally loose:
- `configJson`, `exportStateJson`, and `snapshotTransactionsJson` are still raw
  strings
- `adjustments` is still an untyped string-double map
- `AnalysisPolicy` mostly validates presence and compatibility, not a full
  schema

#### Notes

This entity is already more than a DTO, but it is not yet a fully typed domain
model either. If one area should receive more structure later, it is this one.

### `Annual`

File: `core/include/core/domain/entities/Annual.h`

`Annual` is the yearly product container. It manages ordered analysis ids and
year metadata.

#### State

Current public state:
- `id`
- `name`
- `year`
- `analysisIds`
- `createdAt`
- `updatedAt`

#### Behavior

Implemented methods:
- `rename(EntityName)`
- `rename(std::string)`
- `setYear(Year)`
- `setYear(int)`
- `setAnalysisIds(std::vector<std::string>)`
- `setAnalyses(std::vector<std::string>)`
- `addAnalysisId(std::string)`
- `addAnalysis(std::string)`
- `assignAnalysis(std::string)`
- `insertAnalysisId(std::string, std::size_t position)`
- `removeAnalysisId(const std::string&)`
- `removeAnalysis(std::string)`
- `clearAnalysisIds()`
- `containsAnalysisId(const std::string&) const`
- `containsAnalysis(const std::string&) const`
- `indexOfAnalysisId(const std::string&) const`
- `moveAnalysisId(const std::string&, std::size_t newPosition)`
- `moveAnalysis(std::string, std::size_t newPosition)`
- `analysisCount() const`
- `hasAnalysisIds() const`
- `hasYear() const`
- `isEmpty() const`
- `hasAnalysis(const std::string&) const`

#### Invariants

Currently enforced:
- years are validated through `Year` and `AnnualPolicy`
- analysis ids are normalized and deduplicated
- analysis order is preserved

Still missing:
- explicit annual validation results
- checks that analyses actually match the annual year
- a richer rule for result integrity beyond "ids are present and unique"

#### Notes

`Annual` is structurally nice already. The next meaningful step is not more
state, but a better validation story.

### `WorkspaceState`

File: `core/include/core/domain/WorkspaceState.h`

`WorkspaceState` is the current catalog snapshot. Conceptually, it is not a
classic aggregate root. It is a workspace document that groups multiple domain
collections.

#### State

Current public state:
- `properties`
- `actors`
- `contracts`
- `statements`
- `statementDrafts`
- `transactions`
- `transactionDrafts`
- `analyses`
- `annuals`

#### Behavior

Implemented methods:
- `empty() const`

#### Invariants

It currently only checks that all collections are empty.

#### Notes

This is the least clean part of the current `core/domain` story.
The file lives in the domain namespace, but it still contains workflow-related
draft collections that conceptually belong outside the core catalog model.

The newer application-level `Workspace` type already splits catalog and
workflow more cleanly. `WorkspaceState` should eventually reflect that split or
be replaced by a clearer catalog-only type.

---

## Value Objects

### `Alias`

File: `core/include/core/domain/values/Alias.h`

`Alias` stores the persisted alias record.

#### State

- `value`
- `kind`
- `source`
- `createdAt`
- `updatedAt`

#### Semantics

This type is intentionally simple. It represents the alias payload rather than
the matching rules themselves.

#### Notes

Normalization is not done inside the struct. It is handled by `AliasPolicy`.

### `AliasUsage`

File: `core/include/core/domain/values/AliasUsage.h`

`AliasUsage` stores ranking metadata for aliases.

#### State

- `alias`
- `hitCount`
- `lastUsedAt`
- `updatedAt`
- `createdAt`

#### Semantics

This type is used to keep track of how often and how recently an alias was
used by matching logic.

#### Notes

It is a persistence-friendly value type with ranking metadata, not a separate
domain aggregate.

### `AliasValue`

File: `core/include/core/domain/values/AliasValue.h`

`AliasValue` is the normalized textual alias key.

#### Rules

- trims whitespace
- rejects empty values after normalization
- defines a conservative max length

#### Role

Used as the canonical input when a method needs "an alias key" instead of a
full alias record.

### `EntityName`

File: `core/include/core/domain/values/EntityName.h`

`EntityName` normalizes display names for domain entities.

#### Rules

- trims whitespace
- rejects empty values after normalization
- defines a conservative max length

#### Role

Shared across actor-like and product-like entities to avoid repeated ad hoc
string trimming.

### `ContractType`

File: `core/include/core/domain/values/ContractType.h`

`ContractType` normalizes contract type labels.

#### Rules

- trims whitespace
- rejects empty values after normalization
- defines a conservative max length

#### Role

Provides a stable contract type abstraction for matching and grouping.

### `AnalysisType`

File: `core/include/core/domain/values/AnalysisType.h`

`AnalysisType` normalizes the analysis type label.

#### Rules

- trims whitespace
- rejects empty values after normalization
- defines a shorter max length than generic names

#### Role

Used to distinguish analysis result modes without relying on raw strings.

### `ExportFormat`

File: `core/include/core/domain/values/ExportFormat.h`

`ExportFormat` normalizes the export format label.

#### Rules

- trims whitespace
- rejects empty values after normalization
- defines a shorter max length than generic names

#### Role

Used by analysis export configuration.

### `FilterSpec`

File: `core/include/core/domain/values/FilterSpec.h`

`FilterSpec` stores the normalized textual representation of an analysis
filter specification.

#### Rules

- trims whitespace
- rejects empty values after normalization
- allows a larger payload than simple labels

#### Role

This is the current bridge between a conceptual filter and the persisted
string representation.

### `BookingDate`

File: `core/include/core/domain/values/BookingDate.h`

`BookingDate` normalizes the booking date text used by transactions.

#### Rules

- trims whitespace
- rejects empty values after normalization
- uses a short max length

#### Role

Keeps date-like strings from leaking into transaction logic unnormalized.

### `MoneyAmount`

File: `core/include/core/domain/values/MoneyAmount.h`

`MoneyAmount` is the numeric monetary wrapper used by transactions.

#### Rules

- stores a `double`
- validates via `std::isfinite`

#### Role

This is intentionally light-weight. The domain still uses floating point
storage because the persistence model does, but the wrapper makes the intent
explicit.

### `Year`

File: `core/include/core/domain/values/Year.h`

`Year` normalizes annual year values.

#### Rules

- accepts only values in the range `1..9999`
- normalizes invalid input to `0`
- treats `0` as empty

#### Role

Used by `Annual` and annual validation policies.

---

## Policies

### `AliasPolicy`

File: `core/include/core/domain/policies/AliasPolicy.h`

`AliasPolicy` is the shared alias normalization and ranking helper.

#### Responsibilities

- trim and canonicalize alias values
- normalize alias records
- normalize alias usage records
- deduplicate alias lists
- remove alias values from collections
- record alias hits

#### Key behavior

- if alias value is empty, source is used as fallback
- if source is empty, value is used as fallback
- timestamps are filled on normalization when missing
- duplicate alias values are removed from the normalized collection

#### Notes

This is one of the clearest and best-justified policies in the codebase.
It directly encodes shared business behavior used by multiple entities.

### `AnalysisPolicy`

File: `core/include/core/domain/policies/AnalysisPolicy.h`

`AnalysisPolicy` holds helper logic for analysis configuration and output
typing.

#### Responsibilities

- check whether a type looks valid
- distinguish tabular from chart-like types
- check export compatibility
- decide whether an analysis is configured
- resolve an output type from a type/config pair

#### Notes

This policy is useful, but it is still mostly presence-and-shape checking.
The old plan wanted a richer, typed analysis model eventually, and that still
remains a valid refactor target.

### `AnnualPolicy`

File: `core/include/core/domain/policies/AnnualPolicy.h`

`AnnualPolicy` contains validation helpers for annual collections.

#### Responsibilities

- validate year range
- normalize ids
- check for duplicates
- check membership
- check whether all required ids are present

#### Notes

This is a good base, but it does not yet express higher-level annual integrity
rules such as year-vs-analysis consistency.

### `TransactionPolicy`

File: `core/include/core/domain/policies/TransactionPolicy.h`

`TransactionPolicy` contains helpers for transaction normalization and draft
finalization.

#### Responsibilities

- trim transaction-related text
- validate booking dates
- validate amounts
- validate statement assignment
- normalize ids
- deduplicate id collections
- determine whether a transaction can be finalized from draft
- determine whether status can advance

#### Notes

This policy already carries important business meaning.
It is also the place where draft finalization logic is currently anchored,
which is acceptable for now but still a sign that domain and workflow are not
fully separated yet.

### `DraftMatchingPolicy`

File: `core/include/core/domain/policies/DraftMatchingPolicy.h`

`DraftMatchingPolicy` is the import-time matching helper collection.

#### Responsibilities

- trim and normalize import text
- split and join text for matching
- detect metadata boundaries and actor continuation signals
- compute token overlap and alias weights
- extract type and actor text from statement metadata
- derive known contract types from workspace state

#### Notes

This policy is domain-adjacent rather than pure core domain. It is useful, but
it also shows why the old plans insisted that workflow and import models should
stay out of the core entity set.

---

## Cross-Cutting Invariants

The following invariants are consistently used across the current model:

- trim textual input before storage
- reject duplicate ids inside ordered membership lists
- keep alias matching logic centralized
- keep status progression explicit
- keep year validation bounded and predictable
- treat ordered ids as meaningful, not incidental
- avoid mixing import workflow records with the actual catalog model

Some of these are only partially enforced today because public state still
allows direct mutation.

---

## Refactor Points

This section is intentionally compact, but concrete. It collects the remaining
items from the former two plans that are still worth tracking.

### High Priority

1. Split `WorkspaceState` into a catalog-only model and a separate workflow
   model, or rename it to match its real responsibility.
2. Remove the remaining conceptual overlap between `core/domain` and
   application-level draft/log workflow state.
3. Reduce direct public mutation on domain entities so normalization and
   invariants cannot be bypassed.
4. Decide the source of truth for `Statement` and `Transaction` membership,
   then encode that rule clearly instead of leaving it implicit.

### Medium Priority

5. Add a dedicated `StatementPolicy` if statement ordering and ownership rules
   should be reused outside the entity.
6. Tighten `Transaction` finalization semantics so draft cleanup is clearly
   distinguished from regular domain mutation.
7. Keep `Analysis` on a path toward typed config/output objects instead of
   leaving JSON fields as the long-term shape.
8. Add an explicit annual validation result model if annual consistency checks
   become user-visible or workflow-critical.
9. Consider stronger encapsulation for `Actor`, `Property`, `Contract`,
   `Statement`, `Transaction`, `Analysis`, and `Annual` if the model should
   become more invariant-driven.

### Lower Priority

10. Introduce typed ids only if they remove real recurring bugs or noise.
11. Replace `double` money handling only if the monetary model becomes precise
    enough to justify the churn.
12. Expand `DraftMatchingPolicy` only if import matching rules continue to grow
    and need a stable reusable rule surface.
13. Revisit whether `Contract` should ever support "one actor only" as a
    product rule; the current schema still allows many-to-many relations.
14. Keep `AnalysisPolicy` and `AnnualPolicy` small unless new cross-entity rules
    actually require more centralized logic.

### Already Good Enough For Now

15. Keep `AliasPolicy` as the shared alias normalization source of truth.
16. Keep `EntityName`, `AliasValue`, `ContractType`, `AnalysisType`,
    `ExportFormat`, `FilterSpec`, `BookingDate`, `MoneyAmount`, and `Year`
    instead of replacing them with raw strings again.
17. Do not move workflow draft and log models back into `core/domain` entities.

## Bottom Line

The current domain is not "done", but it is solid.

The biggest remaining quality issue is not missing DDD ceremony. It is the
mixing of catalog domain and workflow state in `WorkspaceState`, plus the fact
that many entities still expose public mutable fields.

If the next refactor wave is kept disciplined, the best payoff will come from:
- separating workspace catalog from workflow records
- tightening ownership and membership rules
- keeping the current value-object and policy direction, instead of widening
  the model unnecessarily
