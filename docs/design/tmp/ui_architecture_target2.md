# UI Zielarchitektur

Diese Notiz beschreibt den aktuellen Zielzustand fuer den `ui`-Refactor.
Sie ersetzt die fruehere Uebergangsskizze: keine Alias-Bruecken, keine
Kompatibilitaetsdateien, keine parallelen alten Pfade.

## Leitbild

- QML konsumiert UI-State-Objekte als Presenter fuer Views.
- Diese Presenter orchestrieren Workflows, wenn ein Use Case Arbeit ausloest.
- Dauerhafte Daten, Mutationen und Snapshot-Refresh laufen ueber
  `ui/workspace/WorkspaceFacade`.
- `ui/src`, `ui/include/ui` und `ui/tests` konsumieren nur `core/ports`, nie
  `core/application` oder `core/domain`.
- Die State-Schicht konsumiert keine Feature-Ports wie analysis, annual,
  import oder export. Sie darf Workspace verwenden und Workflows ansteuern.
- Workflows sind keine QML-Oberflaeche. Sie haben keine `Q_PROPERTY` und werden
  nicht direkt aus QML konsumiert.
- Booking gehoert zum Catalog-Bereich, nicht in eigene `booking`-State- oder
  ViewModel-Ordner.

## Aktuelle Struktur

```text
ui/
  include/
    MainWindow.h
    ui/
      shell/
        AppActions.h
        AppContext.h
        QmlContracts.h
        QmlRuntime.h
        window/
          CloseWorkflow.h
          DropHandler.h
          MainWindowContext.h
          MainWindowTrace.h
      workspace/
        WorkspaceFacade.h
      workflows/
        analysis/
          AnalysisWorkflow.h
        annual/
          AnnualWorkflow.h
        export/
          ExportRunner.h
          ExportWorkflow.h
        import/
          DraftViewMapper.h
          ImportDraftMapper.h
          ImportJobBridge.h
          ImportWorkflow.h
          ImportWorkflowState.h
          ImportWorkflowSupport.h
      state/
        catalog/
          ActorState.h
          CatalogFormState.h
          BookingState.h
          CatalogStateSupport.h
          ContractState.h
          LinkedCatalogState.h
          PropertyState.h
        export/
          ExportState.h
          ExportStateSupport.h
        filters/
          FilterState.h
        import/
          ImportState.h
          StatementDraftState.h
          TransactionDraftState.h
          TransactionDraftStateSupport.h
        navigation/
          NavigationState.h
        reporting/
          AnalysisState.h
          AnalysisStateSupport.h
          AnnualState.h
          AnnualStateSupport.h
          ReportingStateSupport.h
        selection/
          RowSelectionSupport.h
          SelectionState.h
          SelectionStateRefreshSupport.h
        session/
          SessionMutationState.h
          SessionMutationStateSupport.h
          WorkspaceSessionModels.h
          WorkspaceSessionSelection.h
          WorkspaceSessionState.h
        settings/
          SettingsState.h
          SettingsStateSupport.h
        status/
          StatusState.h
      viewmodels/
        base/
          IndexedListModel.h
          RowListModel.h
          WorkflowRunListModel.h
        catalog/
          ActorListModel.h
          ContractListModel.h
          PropertyListModel.h
          StatementListModel.h
          TransactionFilterModel.h
          TransactionListModel.h
        export/
          ExportRunListModel.h
        import/
          ImportRunListModel.h
          ImportSuggestionViewModel.h
          StatementDraftViewModel.h
          TransactionDraftListModel.h
          TransactionDraftViewModel.h
        reporting/
          AnalysisListModel.h
          AnnualListModel.h
        system/
          SettingsViewModel.h
      adapters/
        WorkspaceRowProjector.h
      platform/
      shared/
        util/
          AmountParsing.h
          Ids.h
          RunMetadata.h
          StringConversions.h
```

## Bewusst entfernte Pfade

- `ui/adapters/core/*`
- `ui/state/booking/*`
- `ui/state/mutation/*`
- `ui/viewmodels/booking/*`
- `ui/workflows/import/ImportRunStore.*`
- `ui/workflows/export/WorkspaceSnapshot.*`
- `AnalysisResultMapper`, `AnnualRequestMapper`, `AnnualResultMapper`,
  `ImportSuggestionMapper`
- alle Application-Header-Aliases und Kompatibilitaetsbruecken

## Rollen

### State als Presenter

Die Klassen unter `ui/state` bilden die QML-nahe Presenter-Schicht. Sie halten
lokalen Formzustand, leiten View-Aktionen weiter und koordinieren Workflows.
Sie enthalten keine Domain- oder Application-Abhaengigkeiten.
Ihre QML-Oberflaeche exportiert keine Wiring-Dependencies wie Workspace,
Actions, SettingsViewModel, Status oder Workflow-Zeiger. Diese Abhaengigkeiten
werden nur im Bootstrap per C++ gesetzt.

`AppContext` ist nur noch die QML-Aggregation der Presenter und Services, die
QML tatsaechlich konsumiert. Interne Bootstrap-Objekte wie `SettingsViewModel`
bleiben ausserhalb des AppContext.

### WorkspaceFacade

`WorkspaceFacade` ist der zentrale UI-Adapter zu den Workspace-Ports. Catalog,
Booking, Reporting-Daten, Import-Drafts und Run-Logs werden dort persistiert
oder aus dem Snapshot in UI-Modelle geladen.

Die Implementierung ist nach Verantwortungen aufgeteilt:

- `WorkspaceFacade.cpp` enthaelt Konstruktion, Port-Bindung, Snapshot-Refresh
  und Selection/Session-Verdrahtung.
- `WorkspaceFacadeStorage.cpp` enthaelt Dateioperationen.
- `WorkspaceFacadeCatalog.cpp` enthaelt Catalog-, Statement- und
  Transaction-Kommandos.
- `WorkspaceFacadeReporting.cpp` enthaelt Analysis- und Annual-Kommandos.
- `WorkspaceFacadeWorkflow.cpp` enthaelt Statement-Drafts sowie Import- und
  Export-Run-Logs.
- Workspace-nahe Katalog-Identitaetsabfragen, zum Beispiel vorhandene
  Property-/Contract-Signaturen fuer Import-Draft-Choices, bleiben ebenfalls
  in `WorkspaceFacade` statt in Feature-Workflows eigene Snapshot-Suchlogik
  aufzubauen.

### Workflows

Workflows sind Use-Case-Orchestrierung fuer rechen- oder jobartige Aktionen:
Analysis, Annual, Export und Import. Sie duerfen Ports konsumieren, bleiben
aber hinter den State-Presentern und werden nicht an QML exportiert.

Import-Workflow-Implementierungen sind nach Ablaufarten getrennt:

- `ImportWorkflowDraftChoices.cpp` erzeugt Actor/Property/Contract-Choices.
  Es delegiert Workspace-Katalog-Lookups an `WorkspaceFacade`.
- `ImportWorkflowDraftEdits.cpp` schreibt direkte Draft-Edits.
- `ImportWorkflowDraftViewState.cpp` baut Derived View State und
  Auto-Selection-Sync.
- `ImportWorkflowDraftNavigation.cpp` behandelt Draft-Stack-Navigation.
- `ImportWorkflowDraftWorkspace.cpp` persistiert/finalisiert Drafts.
- `ImportWorkflowJobs.cpp` startet Jobs und Queue-Eintraege.
- `ImportWorkflowJobCancellation.cpp` behandelt Cancel/Pause.
- `ImportWorkflowJobTerminal.cpp` behandelt Job-Events und Terminal-Zustaende.
- `ImportWorkflowRuns.cpp` synchronisiert persistierte Run-Logs.

### Adapter

Adapter duerfen echte Boundary-Formen uebersetzen. Globale Adapter bleiben
workspace-nah; Feature-spezifische Mapper liegen beim jeweiligen Workflow.
Sie sollen keine zweite Application-Schicht bilden und keine Parallelpfade
neben Workspace/Workflow aufmachen.

`WorkspaceRowProjector` projiziert nur Workspace-Snapshots in QML-Zeilen.
Generische Row-Auswahl, Ordering und Delete-Reselection liegen in
`state/selection/RowSelectionSupport`, Catalog-Form-Snapshots in
`state/catalog/CatalogStateSupport`.

`viewmodels/base/WorkflowRunListModel` buendelt die gemeinsame Run-Listen-
Mechanik fuer Import und Export. `IndexedListModel::setValueItems` buendelt
Snapshot-Listen, damit Catalog- und Reporting-Modelle keine eigenen
`shared_ptr`-Aufbau-Schleifen duplizieren.

## Architektur-Guards

`ui/tests/unit/TestUiArchitectureGuard.cpp` prueft derzeit:

- UI konsumiert keine non-port-Core-Header.
- Produktions-QML konsumiert Workflows nicht direkt.
- Workflow-Header enthalten keine QML-Properties.
- State konsumiert keine Feature-Ports.
- State-Header exportieren keine Wiring-Dependencies als QML-Properties.
- Globale Adapter bleiben workspace-scoped und enthalten keine Feature-Port-
  Mapper.

Neue Refactor-Schnitte sollen diese Regeln erweitern, nicht umgehen.
