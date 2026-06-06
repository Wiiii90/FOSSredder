# Core Draft Refactor

This document describes the desired direction for import drafts and related
workflow records in `core`.

It is meant to answer one question precisely:

> Should `StatementDraft` and `TransactionDraft` be treated like domain
> entities, or should they remain workflow models in `application`?

The answer is:
- they should remain in `core/application`
- they should not be moved back into `core/domain`
- but they should become more clearly defined workflow models instead of
  loosely shaped containers

## Summary

Drafts are not the same thing as the final domain entities.

They are:
- import-time representations
- UI/workflow-facing state
- intermediate selection and matching results
- material for finalization into real domain objects

That means the current placement in `core/application/import/draft` is
directionally correct.

What is still worth improving is the boundary:
- drafts should be clearly modeled as workflow records
- finalization should use domain rules, not ad hoc copy logic
- the domain entities should remain the source of truth for business
  invariants

## What Drafts Are

### Statement Drafts

`StatementDraft` represents an imported statement container before it becomes
an actual `Statement`.

It typically contains:
- the statement-level name
- ordered transaction drafts
- import timestamps
- identifiers used by the workflow

It is not yet a final domain statement because:
- it may still reflect unresolved import state
- it may contain incomplete or ambiguous transaction data
- it may still need matcher-driven cleanup or selection

### Transaction Drafts

`TransactionDraft` represents an imported transaction row before it becomes an
actual `Transaction`.

It currently contains both:
- final-ish business fields such as name, booking date, amount, status
- workflow/import fields such as metadata, proof image data, matcher text,
  selection flags, and draft-only ids

That mixture is exactly why it should not be promoted to a pure domain entity.

It is not just a transaction with extra fields. It is a transaction in a
particular import state.

## Why Drafts Should Stay in Application

### 1. Drafts include workflow-specific state

Fields like these are not core domain state:
- `proofImageData`
- `metadata`
- matcher text
- selection flags
- `statementDraftId`
- draft ordering / position

Those fields belong to the import workflow, not to the final catalog model.

### 2. Drafts are unstable by nature

Domain entities should model stable business concepts.

Drafts are different:
- they exist only during import and curation
- they may be incomplete
- they may be changed repeatedly before finalization
- they can be discarded without representing business loss

That makes them much better application objects than domain objects.

### 3. The final domain model should stay clean

If drafts were moved into `core/domain`, the domain model would likely absorb:
- OCR/import artifacts
- matcher decisions
- selection state
- proof-image handling
- workflow metadata

That would dilute the boundary and make the core harder to reason about.

## What Should Change Instead

The better direction is not to move drafts into domain, but to make the
transition from draft to domain explicit and structured.

### Target shape

- drafts stay in `core/application/import/draft`
- application orchestrates draft loading, editing, matching, and persistence
- domain entities expose clear mutation methods and invariants
- a finalizer or factory translates draft state into domain state

### Core principle

The draft should answer:
- what did we import?
- what did the matcher infer?
- what did the user select?

The domain entity should answer:
- is this business object valid?
- what state transitions are allowed?
- what invariants must always hold?

## Recommended Draft Architecture

### Layer 1: Draft Record

This is the current `TransactionDraft` / `StatementDraft` shape.

Responsibilities:
- store imported data
- store user selections
- store matcher context
- keep workflow ordering
- remain easy to persist and reload

This layer should remain application-owned.

### Layer 2: Draft Finalization Input

Add an explicit finalization input type when needed.

This type should represent only the data needed to create or mutate the real
domain entity.

Examples:
- imported transaction name
- booking date
- amount
- statement assignment
- actor / contract ids
- property ids
- allocatable flag
- normalized status

This type should not include:
- proof image bytes
- OCR metadata
- workflow ids unless they are required for mapping
- UI-specific selection flags

This layer is the bridge between workflow and domain.

### Layer 3: Domain Entity Finalization

`Transaction` and `Statement` should own the real business rules.

Examples:
- what constitutes a valid statement membership
- when a status can advance
- whether a transaction can be finalized
- which fields are cleared or retained after finalization

The application layer should pass data in, but the entity or policy should make
the rule decision.

## Drafts as Mutations: What Is Good and What Is Not

Your intuition is good here: in some sense, a draft is a mutation of the final
entity.

That idea is useful, but it needs precision.

### Good interpretation

A draft can be seen as:
- a source of change
- a temporary pre-finalization state
- a mutation candidate for a domain entity

This is useful because it encourages:
- explicit finalization
- clear mapping
- business-rule-driven transformation

### Bad interpretation

A draft should not be treated as:
- the actual domain entity with a few extras
- a free-form copy of the final entity
- a place where domain invariants can be skipped

That would create weak boundaries and duplicate rule logic.

## Recommended Finalization Flow

### Import and curate

1. Parse or import raw data into draft objects.
2. Run matcher logic over the drafts.
3. Let the user review and adjust the matched data.
4. Persist draft workflow state in application storage.

### Finalize

1. Convert the draft into a narrow finalization input.
2. Apply domain normalization and validation.
3. Create or update the real domain entities.
4. Let domain policies decide whether the result is valid.
5. Persist the finalized domain objects.

### Important distinction

The finalizer should not be a second domain model.
It should remain a translation layer.

## What Belongs Where

### In `core/application/import/draft`

Keep:
- `StatementDraft`
- `TransactionDraft`
- finalization orchestration
- draft matching and selection logic
- draft persistence mapping

### In `core/domain`

Keep:
- `Statement`
- `Transaction`
- `Contract`
- `Actor`
- `Property`
- `Analysis`
- `Annual`
- policies and value objects

### In `core/application/import`

Keep:
- import use cases
- parsing
- OCR / matcher orchestration
- finalization orchestration
- workflow state management

## DTO Question

Not every application object is a DTO.

### DTO

A DTO is just a boundary carrier for a request, response, or projection.

### Draft

A draft is more than a DTO because it has lifecycle and workflow semantics.

That is why the correct label for `StatementDraft` and `TransactionDraft` is
usually:
- workflow model
- intermediate import model
- application state model

not simply DTO.

## Where Domain Rules Should Move

This is the refactor direction that keeps the draft split useful.

### `Transaction`

Good candidates:
- booking date normalization
- amount normalization
- status advancement rules
- statement assignment rules
- post-finalization cleanup semantics

### `Statement`

Good candidates:
- ordered membership rules
- unique transaction membership
- ownership semantics between statement and transaction

### `Contract`

Good candidates:
- relation normalization
- actor/property id uniqueness
- alias matching behavior

### `Analysis` and `Annual`

Good candidates:
- typed configuration
- validation of result state
- membership and ordering rules

## What Not To Do

1. Do not move draft workflow models back into `core/domain` just because they
   look similar to the final entity.
2. Do not keep duplicating finalization logic in multiple application services.
3. Do not let workflow fields leak into domain entities unless they are truly
   business state.
4. Do not create a second parallel domain model for drafts.
5. Do not replace workflow objects with raw maps or JSON blobs.

## Refactor Priorities

### High Priority

1. Make draft-to-domain translation explicit with a dedicated finalization
   input or finalizer boundary.
2. Ensure the domain owns the final business invariants, not the draft layer.
3. Keep `StatementDraft` and `TransactionDraft` in application, but give them
   clearer workflow semantics and less accidental overlap with final entities.
4. Keep `ImportLog` and `ExportLog` as application workflow/audit records.

### Medium Priority

5. Reduce duplicate mutation logic between draft application code and domain
   entity methods.
6. Introduce more typed finalization inputs where raw strings still obscure the
   rule intent.
7. Clarify source-of-truth semantics for statement membership and transaction
   assignment.
8. Improve names so workflow models are obviously workflow models, not
   half-domain objects.

### Lower Priority

9. Split draft persistence concerns further if the workflow grows.
10. Add richer draft validation only if it becomes user-visible or reusable.
11. Convert more of the finalization pipeline to explicit policies if the rule
    set grows enough to justify it.

## Conclusion

The current placement of drafts in `core/application` is the right default.

The main improvement is not architectural relocation, but semantic clarity:
- drafts are workflow records
- domain entities are final business objects
- policies and finalizers translate between the two

That gives us a clean boundary and still leaves enough room to model the import
workflow properly.
