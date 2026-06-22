# Contributing to FOSSredder

Thanks for helping improve FOSSredder. This guide keeps contribution workflow
small, predictable and reviewable.

## Branching

- `master` is the release line.
- `develop` is the integration line for ongoing work.
- Feature, bugfix, docs, chore and refactor branches should normally start from
  `develop` and target `develop`.
- Hotfix branches start from `master` only when a released version needs an
  urgent fix. Merge the fix back into `develop` afterwards.

## Issues

Use the GitHub issue forms whenever possible:

- `Bug report` for reproducible defects or regressions.
- `Feature request` for new product behavior or workflow changes.
- `Docs` for documentation work.
- `Refactor` for structural improvements that should preserve behavior.
- `Chore` for maintenance, tooling or repository hygiene.
- `Release task` for versioning, packaging checks, release notes or milestone closure.
- `Hotfix` for urgent fixes against released versions.

Good issues state the problem, expected outcome, scope, acceptance criteria and
relevant links or logs.

## Pull Requests

- Keep pull requests focused and reviewable.
- Link the related issue.
- Describe behavior changes, risk and verification.
- Update tests, documentation, test matrices or CI contracts when the change
  affects behavior, architecture, runtime layout or supported languages.
- Prefer clean, squashable commits over mixed-purpose history.

## Engineering Standards

Use [docs/DESIGN.md](docs/DESIGN.md) as the detailed project standard.

Short version:

- Preserve behavior during refactors unless the issue explicitly says otherwise.
- Keep business rules in `core`, persistence in `persistence`, external-library
  integrations in `infra/*`, and presentation state in `ui`.
- Keep QML declarative and route user intent through UI state, workflows or core
  ports.
- Remove dead code instead of moving it into new wrappers or private support files.
- Keep public headers documented when public API changes.

## Quality Checklist

Before opening or merging a pull request, make sure:

- relevant tests or CI jobs were run, or the skipped verification is stated
  clearly
- behavior-changing work includes matching tests where practical
- documentation is updated when setup, architecture, usage or release behavior
  changes
- [docs/appendix/test-matrices.md](docs/appendix/test-matrices.md) is updated when
  test coverage or observable behavior changes
- `ci/package` and `ci/localization` contracts are updated when runtime assets,
  QML modules, translations or OCR models change

## Releases

- Milestones represent version targets.
- `develop` can publish nightly pre-releases for installer validation.
- Stable releases are cut from `master` using `v*` tags.
- Release work should verify versioning, release notes, installer output,
  package layout and localization/OCR assets.

## Questions

If something is unclear, open the closest matching issue type and add enough
context for triage. If the work does not fit an existing template, start with a
discussion or ask for maintainer guidance.
