# UI QML Testing Matrix

## Purpose

This document defines the stable QML-level test matrix for the refactored UI.
It covers user-visible behavior at the QML boundary and stays aligned with the
root UI target architecture in `ui_architecture_target.md`.

The matrices below document the implemented QML surface and should remain
stable across internal source refactors as long as observable behavior stays
the same.

The suite is intentionally family-based: when a visible rule applies to every
entity editor, every sidebar, or every common control, the test should express
that reusable behavior instead of a one-off scene.

## Scope

Included in this matrix:
- `ui/tests/qml/common/*`
- `ui/tests/qml/controls/*`
- `ui/tests/qml/components/*`
- `ui/tests/qml/views/*`
- QML components under `ui/qml/FossRedder/*`

Out of scope:
- C++ source-layer behavior
- `core` domain and application behavior
- persistence and infrastructure adapters
- visual styling changes that do not affect observable behavior

## Target Test Tree

```text
ui/
  tests/
    qml/
      qmltests.cpp
      common/
        Lookup.js
        TestSupport.js
      controls/
        tst_Button.qml
        tst_CheckBox.qml
        tst_DropdownMenu.qml
        tst_Panel.qml
        tst_ProgressBar.qml
        tst_TextField.qml
      components/
        tst_BottomBar.qml
        tst_RunLogList.qml
        tst_Toolbar.qml
      views/
        actor/
          tst_ActorView.qml
          tst_ActorSidebar.qml
          tst_ActorForm.qml
          tst_ActorBottomBar.qml
          tst_ActorContractPanel.qml
        property/
          tst_PropertyView.qml
          tst_PropertyForm.qml
          tst_PropertyBottomBar.qml
          tst_PropertySidebar.qml
          tst_PropertyContractPanel.qml
        contract/
          tst_ContractView.qml
          tst_ContractForm.qml
          tst_ContractSidebar.qml
          tst_ContractBottomBar.qml
          tst_ContractActorsPanel.qml
          tst_ContractPropertiesPanel.qml
          tst_ContractTypePanel.qml
          tst_ContractAllocatablePanel.qml
        annual/
          tst_AnnualView.qml
          tst_AnnualForm.qml
          tst_AnnualBottomBar.qml
          tst_AnnualSidebar.qml
          tst_AnnualAnalysesPanel.qml
          tst_AnnualTransactionsPanel.qml
          tst_AnnualVerificationPanel.qml
        booking/
          tst_BookingView.qml
          tst_BookingStatementView.qml
          tst_BookingStatementForm.qml
          tst_BookingSidebar.qml
          tst_BookingBottomBar.qml
          tst_BookingTransactionView.qml
          tst_BookingTransactionForm.qml
          tst_BookingTransactionActorPanel.qml
          tst_BookingTransactionContractPanel.qml
          tst_BookingTransactionPropertyPanel.qml
          tst_BookingTransactionAllocatablePanel.qml
        analysis/
          tst_AnalysisAllocatableFilter.qml
          tst_AnalysisBottomBar.qml
          tst_AnalysisContractTypeFilter.qml
          tst_AnalysisDateFilter.qml
          tst_AnalysisView.qml
          tst_AnalysisPropertyFilter.qml
          tst_AnalysisSidebar.qml
          tst_AnalysisTransactionsPanel.qml
          tst_AnalysisTableView.qml
          tst_AnalysisPlotView.qml
          tst_AnalysisForm.qml
        import/
          tst_ImportBottomBar.qml
          tst_ImportForm.qml
          tst_ImportHomeView.qml
          tst_ImportPanel.qml
          tst_ImportProgressBar.qml
          tst_ImportSidebar.qml
          tst_ImportView.qml
          tst_StatementDraftBottomBar.qml
          tst_StatementDraftForm.qml
          tst_StatementDraftView.qml
          tst_TransactionDraftContractActorPanel.qml
          tst_TransactionDraftContractAllocatablePanel.qml
          tst_TransactionDraftContractPanel.qml
          tst_TransactionDraftContractPropertyPanel.qml
          tst_TransactionDraftFieldRow.qml
          tst_TransactionDraftForm.qml
          tst_TransactionDraftMetadataPanel.qml
          tst_TransactionDraftProofPanel.qml
          tst_TransactionDraftView.qml
        export/
          tst_ExportBottomBar.qml
          tst_ExportSidebar.qml
          tst_ExportForm.qml
          tst_ExportPanel.qml
          tst_ExportProgressBar.qml
          tst_ExportView.qml
        settings/
          tst_SettingsExport.qml
          tst_SettingsGeneral.qml
          tst_SettingsImport.qml
          tst_SettingsMiscellaneous.qml
          tst_SettingsSidebar.qml
          tst_SettingsView.qml
```

## Testing Principles

- Test the observable QML behavior, not the C++ implementation details behind
  it.
- Keep test ids stable so the matrix remains useful when the UI internals move.
- Prefer matrix-prefixed QML test function names so the code stays easy to cross-reference with the table rows.
- Use the smallest realistic QML harness, fake state, or fake app context that can express
  the interaction.
- Prefer boundary wiring tests over layout pixel tests unless layout behavior is
  the point of the test.
- Keep QML tests separate from source-layer tests so each suite owns a single
  responsibility.
- Prefer repeated behavior families over bespoke one-screen-only assertions
  unless the screen is truly a unique edge case.
- For list-backed views, include a revision-driven refresh assertion so derived
  models rebind when the workspace data changes.

## Import

### ImportView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-V-001 | Import stack composition | QML/Composition | ImportState and theme available | Open ImportView | The import content stack is mounted |

### ImportHomeView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-H-001 | Overview composition | QML/Composition | ImportState and theme available | Open ImportHomeView | Form, file panel, progress bar, and bottom bar are mounted with the same state |

### ImportBottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-V-002 | Import start | QML/Interaction | Bottom bar receives state with `canStart` | Click Start | `ImportState.startImport()` is called |
| IMP-V-003 | Import clear | QML/Interaction | Bottom bar receives state with `canClear` | Click Clear | `ImportState.resetStatus()` is called |
| IMP-V-004 | Running actions | QML/Interaction | Bottom bar receives running state | Click Cancel, Cancel all, Pause | The matching ImportState actions are called once |
| IMP-V-009 | Import draft navigation | QML/Interaction/Layout | Bottom bar receives navigable draft state | Click outer draft navigation buttons | Previous/next draft actions delegate to ImportState |

### ImportPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-P-001 | Manual import path | QML/Interaction | Import panel receives editable state | Edit manual path and activate `+` | State text updates and `commitManualImportFiles()` is called |
| IMP-P-002 | Browse import file | QML/Interaction | Import panel receives editable state | Click Browse | `browseImportPdf()` is called |

### ImportProgressBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-PB-001 | Progress binding | QML | Import progress state is populated | Open progress component | Progress value follows `ImportState.progressValue` |

### ImportForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-F-001 | Source selection | QML | Form receives ImportState | Inspect source selector | Supported import source labels from ImportState are shown |
| IMP-F-002 | Strategy selection | QML | Form receives ImportState | Inspect strategy selector | Supported statement strategy labels from ImportState are shown |

### ImportSidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-S-001 | Run-log binding | QML/Interaction | ImportState exposes persisted logs | Open the sidebar | The restored run-log rows are visible and workspace refresh is requested |
| IMP-S-002 | Draft run navigation | QML/Interaction | Import log row has an attached draft | Click the run row | The row payload is delegated to `ImportState.activateRun(...)` |
| IMP-S-003 | Finalized run navigation | QML/Interaction | Import log row has a finalized statement id | Click the run row | The statement payload is delegated to `ImportState.activateRun(...)` |
| IMP-S-005 | Import log delete | QML/Interaction | Import log row is visible | Click the row delete button | The row payload is delegated to `ImportState.deleteRun(...)` |
| IMP-S-006 | Selected draft log highlight | QML/Visual state | Import sidebar has an active draft id and a matching run-log row | Open the sidebar | The matching draft run is outlined with the theme selection color |

### StatementDraftBottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-D-001 | Draft return | QML/Interaction | Statement draft bottom bar receives draft state | Activate return button | `StatementDraftState.returnToImport()` is called |
| IMP-D-002 | Draft discard | QML/Interaction | Statement draft bottom bar receives draft state | Activate discard button | `StatementDraftState.discardDraft()` is called |
| IMP-D-003 | Draft finalize | QML/Interaction | Statement draft bottom bar receives draft state | Activate finalize button | `StatementDraftState.finalizeDraft()` is called |
| IMP-D-004 | Draft action availability | QML/State | Statement draft state has no draft | Inspect lifecycle actions | Return, discard, and finalize are disabled |
| IMP-D-005 | Transaction and draft navigation | QML/Interaction | Statement draft bottom bar receives navigable state | Activate previous/next buttons | Navigation actions delegate to StatementDraftState |
| IMP-D-007 | Navigation availability | QML/State | Statement draft state marks navigation unavailable | Inspect navigation buttons | Previous/next transaction and draft buttons are disabled |

### StatementDraftForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-D-006 | Transaction delete | QML/Interaction | Statement draft form receives deletable state | Activate delete transaction button | `deleteCurrentTransaction()` is called |
| IMP-D-009 | Statement name and transaction add | QML/Interaction | Statement draft form receives draft state | Edit statement name and activate add transaction | State name updates and `addTransactionAfterCurrent()` is called |

### StatementDraftView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-D-015 | Empty draft state | QML/Composition | StatementDraftView receives no draft | Open StatementDraftView | The internal StatementDraftState reports no draft and the empty-state label is visible |

### TransactionDraftForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-D-010 | Amount edit commit boundary | QML/Interaction | Transaction form receives amount state | Edit amount text without finishing, then finish editing | Local state updates immediately; commit runs only on editing finished |
| IMP-D-011 | Amount value binding | QML/State | Transaction form receives committed amount state | Change `TransactionDraftState.amountText` | Amount field follows the committed state value |
| IMP-D-016 | Basic field binding | QML/Interaction | Transaction form receives editable state | Edit name, date, status, valuta, and amount | Matching TransactionDraftState properties and commit methods are used |

### TransactionDraftMetadataPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-D-008A | Draft metadata rendering | QML/Rendering | TransactionDraftState contains metadata | Open metadata panel | Metadata text is rendered from TransactionDraftState |

### TransactionDraftProofPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-D-008B | Draft proof rendering | QML/Rendering | TransactionDraftState contains a proof image source | Open proof panel | Proof image source is rendered from TransactionDraftState |

### TransactionDraftContractPropertyPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-D-012 | Property selection and quick-create | QML/Interaction | Property panel receives rows and quick-create state | Toggle a property and activate `+` | Property selection and creation delegate to TransactionDraftState |

### TransactionDraftContractPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-D-013 | Contract quick-create | QML/Interaction | Contract panel receives quick-create state | Enter contract name/type and activate `+` | Contract fields update and creation delegates to TransactionDraftState |

### TransactionDraftContractActorPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-D-014 | Actor quick-create | QML/Interaction | Actor panel receives quick-create state | Enter actor name and activate `+` | Actor text updates and creation delegates to TransactionDraftState |
| IMP-D-020 | Actor suggestion label refresh | QML/State sync | Actor panel starts with `0%` suggestion state | Update suggestion confidence and summary on TransactionDraftState | The label text and color rebind to the matcher suggestion values |

### TransactionDraftContractAllocatablePanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-D-017 | Allocatable toggle | QML/Interaction | Allocatable panel receives transaction state | Toggle allocatable state | The allocatable change delegates to TransactionDraftState |

### TransactionDraftFieldRow

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-D-018 | Field row content loading | QML/Layout | Left and right content components are supplied | Open field row | Both content components are loaded into the two-column row |

### TransactionDraftView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-D-019 | Transaction draft composition | QML/Composition | TransactionDraftState and theme available | Open TransactionDraftView | Transaction form and all transaction panels are mounted with the same state |

## Export

### ExportView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-V-001 | Container mount | QML | ExportState and theme available | Open ExportView | Export form, panel, progress bar, and bottom bar mount with the injected ExportState |
| EXP-V-002 | Command wiring | QML/Interaction | ExportState exposes browse and start commands | Click Browse and Start | Commands are delegated to ExportState without duplicating export logic in QML |

### ExportForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-F-001 | Target directory | QML | Form loaded with ExportState | Edit target directory | ExportState target directory updates deterministically |
| EXP-F-002 | Archive format | QML | Form loaded with ExportState | Select archive format | ExportState package format updates deterministically |
| EXP-F-003 | Browse action | QML | Form loaded with ExportState | Activate browse button | Browse command is delegated to ExportState |

### ExportPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-P-001 | Annual export binding | QML/Interaction | Annual and analysis rows available | Open panel in annual mode | Annual entries are shown and add/remove actions stay deterministic |
| EXP-P-002 | Analysis export binding | QML/Interaction | Analysis rows available | Switch to analysis mode | Analysis entries are shown and add/remove actions stay deterministic |
| EXP-P-003 | Remove entry command | QML/Interaction | Panel has an export entry | Click the compact remove button | Remove is delegated to ExportState |
| EXP-P-004 | Annual disclosure command | QML/Interaction | Panel has an expanded annual entry | Click the disclosure button | Collapse state is delegated to ExportState |
| EXP-P-005 | Export type command | QML/Interaction | Panel has a standalone analysis entry | Change export type | Export type selection is delegated to ExportState |

### ExportBottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-BB-001 | Create-mode commands | QML/Interaction | ExportState in create mode | Click Clear and Start | Clear and start commands are delegated to ExportState |
| EXP-BB-002 | Progress-mode commands | QML/Interaction | ExportState in progress mode | Click Cancel and Pause | Cancel and pause commands are delegated to ExportState |

### ExportProgressBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-PB-001 | Progress binding | QML | Export workflow running | Inspect the progress bar | Progress value and status text follow the workflow state |
| EXP-PB-002 | Error binding | QML | ExportState exposes an error status | Inspect the progress status | Error text uses the danger color |

### ExportSidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-S-001 | Run-log binding | QML/Interaction | ExportState exposes persisted logs | Open the sidebar | The restored run-log rows are visible in the list and refresh is delegated |

## Actor

### ActorView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-V-003 | Bottom navigation cycles through create mode | QML/Interaction | Multiple actor rows and the last/first actor selected | Click next from the last actor or previous from the first actor, then click again | Selection clears into create mode first, then continues to the opposite edge |
| ACT-V-004 | Bottom navigation starts from create mode | QML/Interaction | Multiple actor rows and no actor selected | Click next or previous | Next selects the first actor and previous selects the last actor |
| ACT-V-005 | Bottom navigation supports single row | QML/Interaction | Actor form opened with only one actor row | Click previous or next | Previous/next buttons remain enabled and navigation can select the only actor |

### ActorSidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-S-001 | Row binding | QML/Interaction | Sidebar loaded with actor state rows | Open the sidebar | The sidebar renders the actor row labels from actor state |
| ACT-S-002 | Row selection | QML/Interaction | Sidebar loaded with actor rows | Click an actor row | The selection request is forwarded through actor state |
| ACT-S-003 | Selected highlight | QML/Visual state | Sidebar loaded with a selected actor id | Open the sidebar | The matching actor row uses the theme selection highlight |
| ACT-S-004 | Sidebar scrolling | QML/Layout | Sidebar has more rows than visible height | Open the sidebar | The internal flickable becomes scrollable for the full row list |

### ActorForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-F-001 | Create actor | QML/Interaction | No selected actor | Enter name and click Create | `saveActor("", ...)` is called, form clears, selected actor id becomes returned id |
| ACT-F-002 | Alias add/delete | QML/Interaction | Create or edit mode with alias input available | Activate `+` and `-` alias buttons | Alias is trimmed, added once, removed and index updates deterministically |
| ACT-F-003 | Edit-mode alias add | QML/Interaction | Edit mode with a selected actor | Add an alias | Alias is added to actor state without leaving edit mode |
| ACT-F-004 | Alias layout space | QML/Layout | Actor form rendered with aliases | Open form | Alias panel receives stable layout space |
| ACT-F-004B | Alias chip selection | QML/Interaction | Actor form rendered with multiple aliases | Click an alias chip | Alias index updates and remove action becomes enabled |
| ACT-F-005 | Read actor state | QML/Interaction | Selected actor with name and aliases | Open form in edit mode | Fields reflect selected actor state |
| ACT-F-006 | Update actor | QML/Interaction | Selected actor with modified fields | Change name or aliases and click Update | `saveActor(current.id, ...)` is called and saved state is refreshed |
| ACT-F-007 | Update selected contracts | QML/Interaction | Selected actor with contract rows | Toggle contract checkboxes and update | Selected contract ids are saved |
| ACT-F-008 | Selection signal refresh | QML/State sync | Selected actor object emits changed | Change selected aliases externally | Form state refreshes without replacing the selection object |
| ACT-F-009 | Data revision refresh | QML/State sync | Selected actor object does not emit changed | Trigger data revision | Form state refreshes from selected actor data |
| ACT-F-010 | Contract selection | QML/Interaction | Selected actor with contract rows | Toggle contract checkboxes | Selected contract ids are updated in the form state |
| ACT-F-011 | Actor navigation | QML/Interaction | Actor rows available | Click Prev or Next | Selected actor id moves to adjacent actor |
| ACT-F-012 | Create-mode shortcut button | QML/Interaction | Edit mode with a selected actor | Click bottom-bar `+` button | Selection clears and form switches to create mode |
| ACT-F-013 | Delete actor | QML/Interaction | Selected actor with valid id | Click Delete | `deleteActor(id)` is called and selection advances deterministically |

### ActorBottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-BB-001 | Navigation buttons | QML/Interaction | Bottom bar loaded with actor rows and state hooks | Click Prev or Next | Navigation methods are called on the actor state |
| ACT-BB-002 | Create-mode buttons | QML/Interaction | Bottom bar loaded in create mode | Click Clear or Create | Clear and submit actions are forwarded to the actor state |
| ACT-BB-003 | Edit-mode buttons | QML/Interaction | Bottom bar loaded in edit mode with changes | Click `+`, Delete or Update | Create-mode, delete and update actions are forwarded to the actor state |

### ActorContractPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-CP-001 | Contract selection | QML/Interaction | Actor contract panel loaded with contract rows | Toggle a contract checkbox | The selected contract ids on actor state are updated |
| ACT-CP-002 | Selection rendering | QML/Visual state | Actor contract panel loaded with a selected contract id | Open the panel | The matching contract checkbox reflects the selected state |

## Property

### PropertyView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-V-003 | Bottom navigation cycles through create mode | QML/Interaction | Multiple property rows and the last/first property selected | Click next from the last property or previous from the first property, then click again | Selection clears into create mode first, then continues to the opposite edge |
| PROP-V-004 | Bottom navigation starts from create mode | QML/Interaction | Multiple property rows and no property selected | Click next or previous | Next selects the first property and previous selects the last property |
| PROP-V-005 | Bottom navigation supports single row | QML/Interaction | Property form opened with only one property row | Click previous or next | Previous/next buttons remain enabled and navigation can select the only property |
| PROP-V-006 | Contract selection enables update | QML/Interaction | Property form opened in edit mode with a changed contract selection | Toggle a contract checkbox | The update action becomes enabled once the form has changes |

### PropertySidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-S-001 | Row binding | QML/Interaction | Sidebar loaded with property state rows | Open the sidebar | The sidebar renders the property row labels from property state |
| PROP-S-002 | Row selection | QML/Interaction | Sidebar loaded with property rows | Click a property row | The selection request is forwarded through property state |
| PROP-S-003 | Selected highlight | QML/Visual state | Sidebar loaded with a selected property id | Open the sidebar | The matching property row uses the theme selection highlight |
| PROP-S-004 | Sidebar scrolling | QML/Layout | Sidebar has more rows than visible height | Open the sidebar | The internal flickable becomes scrollable for the full row list |
| PROP-S-005 | Row refresh | QML/State sync | Sidebar is open and property rows change | Update the workspace property rows | The sidebar renders the new property row without reopening |

### PropertyBottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-BB-001 | Navigation buttons | QML/Interaction | Bottom bar loaded with property rows and state hooks | Click Prev or Next | Navigation methods are called on the property state |
| PROP-BB-002 | Create-mode buttons | QML/Interaction | Bottom bar loaded in create mode | Click Clear or Create | Clear and submit actions are forwarded to the property state |
| PROP-BB-003 | Edit-mode buttons | QML/Interaction | Bottom bar loaded in edit mode with changes | Click `+`, Delete or Update | Create-mode, delete and update actions are forwarded to the property state |

### PropertyForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-F-001 | Create property | QML/Interaction | No selected property | Enter name and click Create | `saveProperty("", ...)` is called, form clears, selected property id becomes returned id |
| PROP-F-002 | Read property state | QML/Interaction | Selected property with name and aliases | Open form in edit mode | Fields reflect selected property state |
| PROP-F-005 | Alias add/delete | QML/Interaction | Create or edit mode with alias input available | Activate `+` and `-` alias buttons | Alias is trimmed, added once, removed and index updates deterministically |
| PROP-F-005B | Edit-mode alias add | QML/Interaction | Edit mode with a selected property | Add an alias | Alias is added to property state without leaving edit mode |
| PROP-F-005C | Alias layout space | QML/Layout | Property form rendered with aliases | Open form | Alias panel receives stable layout space |
| PROP-F-005D | Alias chip selection | QML/Interaction | Property form rendered with multiple aliases | Click an alias chip | Alias index updates and remove action becomes enabled |
| PROP-F-006 | Update property | QML/Interaction | Selected property with modified fields | Change name or aliases and click Update | `saveProperty(current.id, ...)` is called and saved state is refreshed |
| PROP-F-007 | Contract selection | QML/Interaction | Selected property with contract rows | Toggle contract checkboxes | Selected contract ids are updated in the form state |
| PROP-F-008 | Update selected contracts | QML/Interaction | Selected property with contract rows | Toggle contract checkboxes and update | Selected contract ids are saved |
| PROP-F-010 | Selection signal refresh | QML/State sync | Selected property object emits changed | Change selected aliases externally | Form state refreshes without replacing the selection object |
| PROP-F-011 | Data revision refresh | QML/State sync | Selected property object does not emit changed | Trigger data revision | Form state refreshes from selected property data |
| PROP-F-012 | Property navigation | QML/Interaction | Property rows available | Click Prev or Next | Selected property id moves to adjacent property |
| PROP-F-013 | Create-mode shortcut button | QML/Interaction | Edit mode with a selected property | Click bottom-bar `+` button | Selection clears and form switches to create mode |
| PROP-F-014 | Delete property | QML/Interaction | Selected property with valid id | Click Delete | `deleteProperty(id)` is called and selection advances deterministically |

### PropertyContractPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-CP-001 | Contract selection | QML/Interaction | Property contract panel loaded with contract rows | Toggle a contract checkbox | The selected contract ids on property state are updated |
| PROP-CP-002 | Selection rendering | QML/Visual state | Property contract panel loaded with a selected contract id | Open the panel | The matching contract checkbox reflects the selected state |

## Contract

### ContractView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-V-003 | Bottom navigation cycles through create mode | QML/Interaction | Multiple contract rows and the last/first contract selected | Click next from the last contract or previous from the first contract, then click again | Selection clears into create mode first, then continues to the opposite edge |
| CON-V-004 | Bottom navigation starts from create mode | QML/Interaction | Multiple contract rows and no contract selected | Click next or previous | Next selects the first contract and previous selects the last contract |
| CON-V-005 | Bottom navigation supports single row | QML/Interaction | Contract form opened with only one contract row | Click previous or next | Previous/next buttons remain enabled and navigation can select the only contract |

### ContractSidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-S-001 | Row binding | QML/Interaction | Sidebar loaded with contract state rows | Open the sidebar | The sidebar renders the contract row labels from contract state |
| CON-S-002 | Row selection | QML/Interaction | Sidebar loaded with contract rows | Click a contract row | The selection request is forwarded through contract state |
| CON-S-003 | Selected highlight | QML/Visual state | Sidebar loaded with a selected contract id | Open the sidebar | The matching contract row uses the theme selection highlight |
| CON-S-004 | Sidebar scrolling | QML/Layout | Sidebar has more rows than visible height | Open the sidebar | The internal flickable becomes scrollable for the full row list |

### ContractBottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-BB-001 | Navigation buttons | QML/Interaction | Bottom bar loaded with contract rows and state hooks | Click Prev or Next | Navigation methods are called on the contract state |
| CON-BB-002 | Create-mode buttons | QML/Interaction | Bottom bar loaded in create mode | Click Clear or Create | Clear and submit actions are forwarded to the contract state |
| CON-BB-003 | Edit-mode buttons | QML/Interaction | Bottom bar loaded in edit mode with changes | Click `+`, Delete or Update | Create-mode, delete and update actions are forwarded to the contract state |

### ContractForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-F-001 | Create contract | QML/Interaction | No selected contract, a valid contract type, and at least one relation selected | Enter name and type, select at least one actor or property, then click Create | `saveContract("", ...)` is called, form clears, selected contract id becomes returned id |
| CON-F-002 | Alias add/delete | QML/Interaction | Create or edit mode with alias input available | Activate `+` and `-` alias buttons | Alias is trimmed, added once, removed and index updates deterministically |
| CON-F-003 | Edit-mode alias add | QML/Interaction | Edit mode with a selected contract | Add an alias | Alias is added to contract state without leaving edit mode |
| CON-F-004 | Alias layout space | QML/Layout | Contract form rendered with aliases | Open form | Alias panel receives stable layout space |
| CON-F-004B | Alias chip selection | QML/Interaction | Contract form rendered with multiple aliases | Click an alias chip | Alias index updates and remove action becomes enabled |
| CON-F-005 | Read contract state | QML/Interaction | Selected contract with type, actors, properties, aliases | Open form in edit mode | Fields and selection panels reflect selected contract state |
| CON-F-006 | Update contract | QML/Interaction | Selected contract with modified fields | Change name, type, selections or aliases and click Update | `saveContract(current.id, ...)` is called and saved state is refreshed |
| CON-F-007 | Update selected relations | QML/Interaction | Selected contract with actor and property rows | Change actor/property selections and update | Selected relation ids are saved |
| CON-F-008 | Replace primary actor | QML/Interaction | Selected contract has an assigned actor | Select a different primary actor | Previous actor assignment is replaced |
| CON-F-009 | Selection signal refresh | QML/State sync | Selected contract object emits changed | Change selected aliases externally | Form state refreshes without replacing the selection object |
| CON-F-010 | Data revision refresh | QML/State sync | Selected contract object does not emit changed | Trigger data revision | Form state refreshes from selected contract data |
| CON-F-011 | Actor selection | QML/Interaction | Actor rows available | Select the primary actor | Selected actor ids are updated in the form state |
| CON-F-012 | Property selection | QML/Interaction | Property rows available | Toggle contract property checkboxes | Selected property ids are updated in the form state |
| CON-F-013 | Relation validation | QML/Interaction | Create mode with no relation selected | Enter name and type | Create remains disabled until an actor or property is selected |
| CON-F-014 | Contract navigation | QML/Interaction | Contract rows available | Click Prev or Next | Selected contract id moves to adjacent contract |
| CON-F-015 | Create-mode shortcut button | QML/Interaction | Edit mode with a selected contract | Click bottom-bar `+` button | Selection clears and form switches to create mode |
| CON-F-016 | Delete contract | QML/Interaction | Selected contract with valid id | Click Delete | `deleteContract(id)` is called and selection advances deterministically |

### ContractActorsPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-AP-001 | Actor selection | QML/Interaction | Contract actor panel loaded with actor rows and contract state | Select an actor from the dropdown | The selected actor id is written through `selectPrimaryActor()` |
| CON-AP-002 | Existing selection rendering | QML/Visual state | Contract actor panel loaded with a selected actor id | Open the panel | The dropdown reflects the selected actor |

### ContractPropertiesPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-PP-001 | Property selection | QML/Interaction | Contract properties panel loaded with property rows | Toggle a property checkbox | The selected property ids on contract state are updated |
| CON-PP-002 | Selection rendering | QML/Visual state | Contract properties panel loaded with a selected property id | Open the panel | The matching property checkbox reflects the selected state |

### ContractTypePanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-TP-001 | Type edit | QML/Interaction | Contract type panel loaded with contract state | Edit the type field | The type is written to contract state |
| CON-TP-002 | Type rendering | QML/Visual state | Contract type panel loaded with an existing type | Open the panel | The type field reflects contract state |

### ContractAllocatablePanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-ALP-001 | Allocatable mode edit | QML/Interaction | Contract allocatable panel loaded with contract state | Select an allocatable mode | The allocatable mode is written to contract state |
| CON-ALP-002 | Allocatable mode rendering | QML/Visual state | Contract allocatable panel loaded with an existing mode | Open the panel | The dropdown reflects contract state |

## Annual

### AnnualView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-V-001 | Container mount | QML | AnnualState and theme available | Open AnnualView | `AnnualForm` and `AnnualBottomBar` mount against the injected AnnualState |
| ANN-V-002 | Bottom-bar command delegation | QML/Interaction | AnnualView mounted in create mode | Click Create in the bottom bar | The command is forwarded to AnnualState |

### AnnualBottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-BB-001 | Create-mode actions | QML/Interaction | AnnualBottomBar mounted with create-mode AnnualState | Click previous, workspace, clear, create, and next | The matching AnnualState methods are called |
| ANN-BB-002 | Edit-mode actions | QML/Interaction | AnnualBottomBar mounted with edit-mode AnnualState | Click delete and update | Delete and update are forwarded to AnnualState |

### AnnualSidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-SB-001 | Row selection | QML/Interaction | Annual rows available in AnnualState | Click a sidebar row | AnnualState receives the selected annual id |

### AnnualForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-F-001 | Name and year binding | QML/Interaction | AnnualForm mounted with AnnualState | Edit name and year controls | AnnualState receives the new values |
| ANN-F-002 | Workspace stack binding | QML/Interaction | AnnualForm mounted with analysis and transaction workspaces | Change AnnualState workspace index | The form switches from analysis assignment to annual transactions |

### AnnualAnalysesPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-P-001 | Add annual analysis | QML | Analysis rows available | Activate add analysis button | Analysis id is added once to selection |
| ANN-P-002 | Export format update | QML/Interaction | Assigned analysis row visible | Change export format | AnnualState receives the target analysis id and export format |
| ANN-P-003 | Remove analysis | QML/Interaction | Assigned analysis visible | Activate remove analysis button | Analysis id is removed from selection |

### AnnualTransactionsPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-T-001 | Category sections render deterministically | QML/Interaction | Grouped annual transaction payload contains visible section rows | Open annual transactions panel | All annual transaction category section toggles are visible |
| ANN-T-002 | Section toggle delegation | QML/Interaction | Annual transaction section visible | Click a section toggle | AnnualState receives the section key |

### AnnualVerificationPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-VP-001 | Verification summary rendering | QML | AnnualState exposes verification rows and status summary | Open AnnualVerificationPanel | The status summary and verification panel mount from AnnualState |

## Booking

### BookingView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-V-001 | Create-mode container mount | QML | BookingState and theme available with no selected statement | Open BookingView | `BookingStatementView` is filled with BookingState and theme in create mode |
| BKG-V-002 | Edit-mode container mount | QML | BookingState and theme available in edit mode | Open BookingView and click Update | The update command is forwarded to `BookingState.updateCurrent()` |

### BookingBottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-BB-001 | Navigation requests | QML/Interaction | Bottom bar has statement and transaction navigation enabled | Click previous/next statement and transaction buttons | The matching navigation methods are called on `BookingState` |
| BKG-BB-002 | Create mode commands | QML/Interaction | Bottom bar is in create mode and create is enabled | Click Clear and Create | `BookingState.resetCreateState()` and `submit()` are called |
| BKG-BB-003 | Edit mode commands | QML/Interaction | Bottom bar is in edit mode and update is enabled | Click Delete and Update | `BookingState.deleteCurrentStatement()` and `updateCurrent()` are called |

### BookingStatementView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-SV-001 | Statement name state binding | QML/Interaction | BookingStatementView receives a BookingState object | Edit statement name field | `BookingState.statementName` receives the edited text |
| BKG-SV-002 | Transaction add/delete state commands | QML/Interaction | BookingStatementView receives a BookingState object with transaction commands | Click add and delete transaction buttons | `BookingState.addTransactionAfterCurrent()` and `deleteCurrentTransaction()` are called |

### BookingStatementForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-SF-001 | Statement name state binding | QML/Interaction | Form loaded with BookingState | Edit statement name field | `BookingState.statementName` is updated directly |
| BKG-SF-002 | Transaction command binding | QML/Interaction | Form loaded with transaction add/delete buttons enabled | Click add and delete transaction buttons | Transaction commands are called on `BookingState` |
| BKG-SF-003 | Transaction view composition | QML | Form loaded with BookingState | Open the form | `BookingTransactionView` is mounted inside the statement panel and receives the same state |

### BookingSidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-S-001 | Transaction row click | QML/Interaction | Statement rows and transaction rows available | Click a transaction row | The matching statement and transaction ids become active |
| BKG-S-002 | Statement row click | QML/Interaction | Statement rows and transaction rows available | Click a statement row | The statement remains selected and the transaction selection is cleared |
| BKG-S-003 | Statement row refresh | QML/Interaction | Statement rows and transaction rows available | Replace the statement row model | Removed transaction rows disappear from the sidebar |

### BookingTransactionView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TV-001 | Transaction view composition | QML | BookingState loaded | Open the transaction view | Transaction form, actor, contract, property, and allocatable panels are mounted with the same state |
| BKG-T-006 | Contract selection cascade | Unit | Covered by `BKG-ST-007` in the UI source matrix | Select contract in transaction | BookingState synchronizes related actor/property ids |
| BKG-T-007 | Actor incompatibility cleanup | Unit | Covered by `BKG-ST-008` in the UI source matrix | Select incompatible actor | BookingState clears incompatible contract id |
| BKG-T-008 | Property incompatibility cleanup | Unit | Covered by `BKG-ST-009` in the UI source matrix | Select incompatible property set | BookingState clears incompatible contract id |

### BookingTransactionForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TF-001 | Transaction field edit | QML/Interaction | BookingState loaded | Edit name, status, booking date, valuta, and amount fields | Matching BookingState transaction properties update |

### BookingTransactionActorPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TA-001 | Actor selection list | QML | BookingState actor display rows available | Select an actor | Selected actor index is forwarded to BookingState |

### BookingTransactionContractPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TC-001 | Contract selection list | QML | BookingState contract display rows available | Select a contract | Selected contract index is forwarded to BookingState |

### BookingTransactionPropertyPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TP-001 | Property selection list | QML | BookingState property rows available | Select a property | Property selection is forwarded to BookingState |
| BKG-TP-002 | Property selection refresh | QML | BookingState selected property ids change | Refresh the selected property ids | The checkbox checked state is derived from BookingState |

### BookingTransactionAllocatablePanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TA-002 | Allocatable mode | QML | BookingState loaded | Change allocatable mode | Mode is written to BookingState |

## Analysis

### AnalysisView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-V-001 | Container refresh | QML/Composition | Analysis state and theme available | Open AnalysisView | `AnalysisState.refreshFromSelection()` is called and the form/bottom bar receive the same state |
| ANL-V-002 | Bottom bar wiring | QML/Interaction | AnalysisView receives an injected analysis state | Click Create through the mounted bottom bar | The command reaches the same AnalysisState instance used by the form |

### AnalysisBottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-BB-001 | Create mode actions | QML/Interaction | Bottom bar receives a create-mode AnalysisState with submit enabled | Click Reset, Create, previous, next, and filter workspace toggle | The matching AnalysisState commands are called |
| ANL-BB-002 | Edit mode actions | QML/Interaction | Bottom bar receives an edit-mode AnalysisState with submit enabled | Click Delete, Update, previous, and next | The matching AnalysisState commands are called |

### AnalysisSidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-SB-001 | Sidebar row selection | QML/Interaction | Sidebar receives analysis rows and selected id state | Click a rendered analysis row | `AnalysisState.selectAnalysis()` receives the row id |

### AnalysisForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-F-001 | Name state binding | QML/Interaction | Form receives AnalysisState | Edit the name field | `AnalysisState.name` receives the edited text |
| ANL-F-002 | Filter panel forwarding | QML/Interaction | Form receives AnalysisState with property and contract filters | Toggle property, contract type, and allocatable filter controls | Filter changes are delegated to AnalysisState |
| ANL-F-003 | Edit result panel | QML/Composition | Form receives edit-mode AnalysisState with rendered result data | Open the form | The result preview panel is mounted from state |
| ANL-F-004 | Include-calc toggle | QML/Interaction | Form receives edit-mode AnalysisState with calc adjustments enabled | Toggle Include Calc Adjustments | `AnalysisState.includeCalcAdjustments` receives the checkbox value |

### AnalysisDateFilter

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-DF-001 | Date controls write state | QML/Interaction | Date filter receives AnalysisState with initial year mode values | Change date field, mode, and range values | Date filter changes are delegated to AnalysisState |
| ANL-DF-002 | Year field state binding | QML/Interaction | Date filter receives AnalysisState with a year value | Edit the year field | `AnalysisState.yearValue` receives the edited value |

### AnalysisPropertyFilter

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-PF-001 | Property filter actions | QML/Interaction | Property filter receives AnalysisState with property rows and selected ids | Click None, All, and one row checkbox; Unassigned remains a regular checkbox row | Selection commands are delegated to AnalysisState |

### AnalysisContractTypeFilter

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-CTF-001 | Contract type filter actions | QML/Interaction | Contract type filter receives AnalysisState with available types and selected types | Click None, All, and one row checkbox; Unassigned remains a regular checkbox row | Selection commands are delegated to AnalysisState |

### AnalysisAllocatableFilter

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-AF-001 | Allocatable mode selection | QML/Interaction | Allocatable filter receives AnalysisState and mode | Change the allocatable dropdown index | The selected mode index is delegated to AnalysisState |

### AnalysisTransactionsPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-TP-001 | Transaction selection and calc commands | QML/Interaction | Panel receives AnalysisState with preview transactions | Select transactions and apply calculation | Selection and calculation requests are delegated to AnalysisState |

### AnalysisPlotView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-PV-001 | Rendered artifact preview | QML | Plot view receives AnalysisState with rendered preview source | Open plot view | The preview image uses the state-provided rendered artifact |

### AnalysisTableView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-TV-001 | Table render uses state matrix | QML | Table view receives contract columns, property rows, and grand total | Open table view | Rows, contract columns, and totals render from AnalysisState |
| ANL-TV-002 | Table preview geometry | QML/Layout | Table view receives a non-empty state matrix | Open table view | Table content exposes renderable geometry for rows and totals |

## Settings

### SettingsView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SET-V-001 | Settings stack mount | QML/State | Settings view receives SettingsState | Mount view | The category stack follows SettingsState and activation is delegated once |
| SET-V-002 | Category navigation | QML/Interaction | Settings view loaded | Click next and previous category controls | Settings category advances and returns with loaded page content |
| SET-V-003 | Update settings | QML/Interaction | Settings view receives SettingsState | Click Update | Save is delegated to SettingsState |
| SET-V-004 | Reset settings | QML/Interaction | Settings category is not the default | Click Default | Settings reset and category returns to General |
| SET-V-005 | Category navigation wraps | QML/Interaction | Settings category is at either edge | Click next from the last category or previous from the first category | Category wraps to the opposite edge |

### SettingsGeneral

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SET-G-001 | Language selection | QML/Interaction | General settings receives language options | Select a language | Selection is delegated to SettingsState |
| SET-G-002 | Unavailable language | QML/Interaction | General settings receives an unavailable option | Select the unavailable language | SettingsState keeps the active language unchanged |
| SET-G-003 | Current language binding | QML/State | SettingsState exposes a selected language index | Mount general settings | The dropdown reflects the selected index |
| SET-G-004 | Theme mode selection | QML/Interaction | General settings receives theme mode options | Select a theme mode | Selection is delegated to SettingsState |

### SettingsImport

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SET-I-001 | Default import path | QML/Interaction | Import settings receives SettingsState | Edit default path | The path updates SettingsState |
| SET-I-002 | Browse import path | QML/Interaction | Import settings receives SettingsState | Click Browse | Browse is delegated to SettingsState |
| SET-I-003 | Import pipeline defaults | QML/Interaction | Import settings receives SettingsState | Edit pipeline fields | Poppler, OpenCV, Tesseract, parser, and matcher defaults update SettingsState |

### SettingsExport

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SET-E-001 | Default export directory | QML/Interaction | Export settings receives SettingsState | Edit default output folder | The directory updates SettingsState |
| SET-E-002 | Archive format | QML/Interaction | Export settings receives SettingsState | Select archive format | The archive format updates SettingsState |
| SET-E-003 | XLSX formulas toggle | QML/Interaction | Export settings receives SettingsState | Toggle formula setting | The formula flag updates SettingsState |
| SET-E-004 | Browse export directory | QML/Interaction | Export settings receives SettingsState | Click Browse | Browse is delegated to SettingsState |

### SettingsMiscellaneous

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SET-M-001 | Toolbar visibility flags | QML/Interaction | Miscellaneous settings receives SettingsState | Toggle toolbar entries | Toolbar visibility flags update SettingsState |

### SettingsSidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SET-S-001 | Category row selection | QML/Interaction | Settings sidebar receives category rows | Click a category row | Category selection is delegated to SettingsState |

## Common Controls and Shared Components

### Button

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-B-001 | Click signal | QML | Button loaded | Activate button | Click is emitted and visual state resets |
| CTRL-B-002 | Visual state reset helper | QML | Button pressed state active | Call clearVisualState() | `down` state is reset |
| CTRL-B-003 | Disabled state | QML | Button disabled | Activate button | Control remains non-active and visual state remains stable |

### CheckBox

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-CB-001 | Toggle state | QML | Checkbox loaded | Toggle checkbox | Checked state changes deterministically |
| CTRL-CB-002 | Layout binding | QML | Checkbox in layout | Inspect control | Checkbox fills width and aligns vertically |

### TextField

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-TF-001 | Text binding | QML | TextField loaded | Edit text | Bound text updates immediately |
| CTRL-TF-002 | Focus styling | QML | TextField focused | Inspect border | Focused border styling is applied |

### DropdownMenu

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-DD-001 | Selection change | QML | Model available | Select item | Current index and activated signal update |
| CTRL-DD-002 | Index update | QML | Model contains multiple options | Change current index | Current index updates deterministically |

### Panel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-P-001 | Default content layout | QML | Panel loaded | Inspect container | Content is placed in the panel body |
| CTRL-P-002 | Content spacing | QML | Panel loaded | Inspect layout spacing | Content spacing follows theme binding |

### ProgressBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-PB-001 | Determinate value binding | QML | Value set | Inspect control value | Progress value reflects assigned percentage |
| CTRL-PB-002 | Indeterminate binding | QML | Indeterminate true | Inspect control state | Indeterminate mode is applied |

### BottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-BB-001 | Content hosting | QML | BottomBar loaded | Add child controls | Child controls are laid out in a row |
| CTRL-BB-002 | Theme binding | QML | Theme loaded | Inspect appearance | Radius, background and spacing follow theme |

### Toolbar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-TB-001 | Domain create-mode navigation | QML/Interaction | Toolbar has actor, property and contract selections | Navigate to each domain section through the toolbar | Domain selections are cleared so the target form opens in create mode |
| CTRL-TB-002 | Booking and tool create-mode navigation | QML/Interaction | Toolbar has booking, analysis and annual selections | Navigate to those sections through the toolbar | Target selections are cleared so the target view opens in create mode |
| CTRL-TB-003 | App menu preserves non-booking selection | QML/Interaction | App menu has existing domain and booking selections | Navigate to a section through the menu | Domain selection is preserved while stale booking selection clears outside Booking |

### RunLogList

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-RL-001 | Log click | QML/Interaction | Log model available | Activate log row | Run click signal wiring is available for row interaction |
| CTRL-RL-002 | Delete click | QML/Interaction | Log row available | Activate remove button | Delete click signal wiring is available for row interaction |
| CTRL-RL-003 | Display role rendering | QML | Log model provides display roles | Inspect rendered labels | Time, title and detail text render from model-provided display roles |
| CTRL-RL-004 | Draft id forwarding | QML/Interaction | Clickable draft log row | Activate log row with a real mouse click | Run click signal includes the draft id |
| CTRL-RL-005 | Delegate click geometry | QML/Layout | Log model contains rows | Open run log list | Delegate rows expose a non-zero click height |
| CTRL-RL-006 | Deleted run click guard | QML/Interaction | Log row has no draft or statement target | Open run log list | Row click area is disabled |
| CTRL-RL-007 | Stable detail geometry | QML/Layout | One log has detail text and another does not | Open run log list | Delegate rows keep equal reserved detail height |
