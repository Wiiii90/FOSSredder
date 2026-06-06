# General Rules

These rules describe the general C++ refactor, formatting, and documentation standard used across the project.

## Code Ownership

- Keep changes scoped to the layer and feature being refactored. Do not hide unrelated cleanup inside a feature refactor.
- Preserve behavior unless the change is explicitly requested. Structural cleanup must not silently change persistence, selection, validation, import/export, analysis, annual, settings, or navigation behavior.
- Remove dead code when discovered. Do not move obsolete code into private headers, support folders, or new helper files to keep it out of review.
- Prefer existing project concepts and local patterns over new abstractions.
- Do not create shared helper files just to reduce a few repeated lines. A shared helper must represent a stable concept, not a temporary escape from one refactor.

## Formatting

- Follow the repository C++ format and clang-tidy configuration. Do not invent local formatting exceptions when the project configuration already answers the question.
- Use the repository clang-format style for mechanical formatting and treat clang-tidy findings as the cleanup baseline unless a finding conflicts with an explicit project rule.
- Keep includes minimal and ordered in the style already used by the file.
- Prefer clear names over comments that explain unclear names.
- Use `const` and `[[nodiscard]]` where they clarify API intent.
- Avoid broad catch-all helpers, manager-style classes, and compatibility wrappers.
- Keep functions short enough to read, but do not split code into artificial files or one-line wrappers.

## Doxygen

- Public headers use Doxygen comments for files, classes, public functions, public data structures, and important public enum values.
- Every public header starts with:
  - `@file`
  - `@brief`
- Public functions in headers should include:
  - `@brief`
  - `@param` for each parameter
  - `@return` for non-`void` return values
  - `@throws` only when the function intentionally propagates documented exceptions
- Constructors and destructors should be documented when they are part of the public API or own important dependencies.
- Source files use a file-level `@file` and `@brief`. Do not duplicate full function Doxygen in `.cpp` files when the function is already documented in the header.
- Private helper functions usually do not need Doxygen. Add a short normal comment only when the helper contains non-obvious algorithmic or architectural intent.
- Keep comments factual. Do not use comments to justify temporary hacks; remove the hack or mark it clearly in an issue/task instead.

## Headers And Source Files

- Headers declare the public contract and private member shape. They should not expose implementation helpers unless QML, tests, or another layer genuinely consume them.
- Source files contain implementation detail. Anonymous-namespace helpers are acceptable when they are local to one file and do not become a hidden cross-feature abstraction.
- Do not introduce private headers to hide unwanted complexity during cleanup.
- Do not split one class across artificial `Form`, `Choices`, `Internals`, `Support`, or `Fields` files unless those are real public concepts with independent ownership.

## Validation

- Core validation is authoritative. UI validation is for immediate user experience and must delegate to core validation whenever the rule is business-relevant.
- Validation errors should be structured as field, code, severity, and message when crossing an API boundary.
- Button enablement may use cheap UI checks, but save/update/finalize/export/import commands must still pass through the authoritative core/workflow validation path.

## Observability

- Add debug traces at useful boundaries: ViewModel user intent, workspace command validation/mutation, workflow lifecycle, adapter runner calls, and storage operations.
- Do not add debug traces to entity setters, low-level value-object normalization, or hot loops unless diagnosing a specific issue.
- Trace messages should include stable context such as ids, operation names, counts, statuses, paths, and the first validation issue where useful.

## Refactor Checklist

1. Read the current header and implementation before editing.
2. Search callsites, including QML, before removing or renaming public API.
3. Remove dead wrappers and aliases instead of moving them.
4. Keep behavior equivalent unless the requested refactor explicitly changes behavior.
5. Update Doxygen in public headers as part of the refactor.
6. Keep source comments sparse and useful.
7. Run focused searches after edits for stale names, old files, and duplicate paths.
