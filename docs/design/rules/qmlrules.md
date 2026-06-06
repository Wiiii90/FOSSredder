# QML Rules

These rules describe the frontend refactor standard used for the QML view-family cleanup.

## Architecture

- Keep QML declarative. QML files compose UI, bind state, and emit user intent; domain logic, workflow logic, selection logic, validation rules, persistence decisions, and data transformation belong outside QML.
- Route view behavior through the corresponding state or workflow object from `ui/src`. Do not duplicate backend policies, entity rules, or workflow decisions in QML.
- Avoid compatibility fallbacks and duplicated bindings. A view should have one deterministic state path and one clear source of truth.
- Use required properties for required dependencies. Do not scatter nullable state guards through production views just to make mounting easier.
- Keep function bodies out of QML unless the function is a tiny UI-only adapter and cannot reasonably be expressed declaratively. Prefer moving behavior into `ui/src`.

## View Families

- A feature view is organized as a view family under `ui/qml/FossRedder/Views/<Feature>/`.
- The top-level `<Feature>View.qml` owns the feature composition.
- Sidebars, forms, panels, bottom bars, and repeated view-local pieces are split into focused QML files inside the same view-family folder.
- View-family files should import shared building blocks from `FossRedder.Controls`, `FossRedder.Components`, and `FossRedder.Theme`; they should not reimplement common controls locally.
- Add or update the view-family `qmldir` registration whenever new QML files are introduced.
- Preserve existing behavior during refactors. Layout, selection, creation, update, delete, filtering, navigation, and preview behavior must remain functionally equivalent unless the change is explicitly requested.

## Controls And Components

- `Controls` contains reusable primitive UI controls: buttons, inputs, checkboxes, dropdowns, panels, scrollbars, progress indicators, and similar low-level widgets.
- `Components` contains reusable app-level compositions: shell, toolbar, app menu, sidebar/content routing, bottom bars, log lists, status chips, and similar shared structures.
- Keep `Controls` and `Components` flat at their public QML module boundary. New shared files can be added directly to the respective folder and registered in `qmldir`.
- Prefer shared controls over view-local one-offs when the pattern appears in more than one view family.
- All styling must be theme-driven. Do not introduce magic colors, spacing, sizes, or one-off visual constants in views.
- Add semantic theme properties only when the distinction is meaningful. Remove dead theme values and avoid bridge values that merely duplicate another property without a real semantic reason.

## Layout And Styling

- Use the theme for spacing, padding, dimensions, colors, typography, borders, shadows, and scrollbar behavior.
- Keep panels and forms top-aligned unless a specific control requires different alignment.
- Avoid invisible layout hacks, arbitrary zero overrides, and magic numbers.
- Keep scrollbars and scroll padding consistent across sidebars, inner panels, and full-view scroll areas.
- App-wide polish belongs in shared controls/components or theme values, not in individual views.

## Tests And Docs

- Mirror important QML structures in the QML test tree where practical, especially for view families and shared controls/components.
- Name QML test functions so they can be found from the design test matrices.
- Keep `docs/design/tests` matrices updated when view behavior, controls, components, or test coverage changes.
- Prefer deterministic test setup helpers such as shared lookup/test-support utilities instead of duplicating state logic inside test fakes.
- Tests should verify behavior that can actually regress: state binding, selection display, enabled states, creation/update/delete intent, filtering, and shared control contracts.

## Refactor Checklist

1. Identify the view family, shared controls, and shared components involved.
2. Move non-UI logic out of QML and into the appropriate `ui/src` state or workflow surface.
3. Split large QML files into focused view-family files without changing behavior.
4. Replace local UI patterns with shared `Controls` or `Components` when the pattern is reusable.
5. Register new QML files in the appropriate `qmldir`.
6. Remove stale compatibility paths, duplicate bindings, dead files, and unused theme values.
7. Check layout consistency against already-refactored view families.
8. Update mirrored QML tests and test matrices.
9. Run the fast QML lint/build path and clear stale build/QMLLS artifacts if diagnostics do not match source.
