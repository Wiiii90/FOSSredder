# UI Zielarchitektur

Diese Notiz beschreibt den Zielzustand fuer den `ui`-Refactor in Richtung v0.5
prod-ready. Sie ist keine Uebergangsskizze: keine Alias-Bruecken, keine
Kompatibilitaetsdateien, keine parallelen alten Pfade und kein
`WorkspaceFacade`-Kompatibilitaetsaggregat.

## Leitbild

- QML konsumiert ViewModels und Shell-Services als schmale Presenter fuer Views.
- ViewModels halten lokalen Form- und Praesentationszustand. Sie halten keine
  Workspace-Schattenkopien und implementieren keine Domain-Regeln.
- Dauerhafte Workspace-Daten kommen aus `core::ports::workspace::WorkspaceSnapshot`
  und werden UI-seitig genau einmal im `WorkspaceStore` gehalten.
- Workspace-Mutationen laufen ueber `WorkspaceCommands`.
- Workspace-Read-Modelle fuer ViewModels und QML laufen ueber
  `WorkspaceSelectors`.
- Cross-View-Auswahl laeuft ueber `WorkspaceSelection`.
- QML-Payload-Konstruktion fuer Workspace-Zeilen liegt in `WorkspacePayloads`.
- Multi-Step-Use-Cases laufen ueber Workflows: Import, Export, Analysis und
  Annual.
- `ui/src`, `ui/include/ui` und `ui/tests` konsumieren nur `core/ports`, nie
  `core/application` oder `core/domain`.

## Zielstruktur

Die Workspace-Dateien bleiben im Workspace-Modul. Die Struktur bleibt flach und
vermeidet eine Verteilung ueber viele `ui/src`-Unterordner.

```text
ui/
  include/
    MainWindow.h
    ui/
      adapters/
        AnalysisAdapter.h
        AnnualAdapter.h
        ExportAdapter.h
        ImportAdapter.h
      i18n/
        Text.h
      observability/
        ErrorCodes.h
        Origins.h
        Trace.h
      platform/
        FileDialogs.h
        FileSystemBrowser.h
        LanguageService.h
      presentation/
        PayloadKeys.h
        PayloadMapper.h
      shell/
        AppActions.h
        AppContext.h
        Defaults.h
        NavigationState.h
        QmlContracts.h
        QmlRuntime.h
        Settings.h
        StatusState.h
        window/
          CloseWorkflow.h
          DropHandler.h
          MainWindowContext.h
          MainWindowTrace.h
      util/
        StringConversions.h
      viewmodels/
        ActorViewModel.h
        AnalysisViewModel.h
        AnnualViewModel.h
        BookingViewModel.h
        ContractViewModel.h
        ExportViewModel.h
        ImportViewModel.h
        PropertyViewModel.h
        SettingsViewModel.h
      workflows/
        AnalysisWorkflow.h
        AnnualWorkflow.h
        ExportWorkflow.h
        ImportWorkflow.h
      workspace/
        WorkspaceCommands.h
        WorkspacePayloads.h
        WorkspaceSelection.h
        WorkspaceSelectors.h
        WorkspaceStore.h
  src/
    adapters/
      AnalysisAdapter.cpp
      AnnualAdapter.cpp
      ExportAdapter.cpp
      ImportAdapter.cpp
    observability/
      Trace.cpp
    platform/
      FileDialogs.cpp
      FileSystemBrowser.cpp
      LanguageService.cpp
    shell/
      AppActions.cpp
      AppContext.cpp
      Composition.cpp
      NavigationState.cpp
      QmlRuntime.cpp
      Settings.cpp
      window/
        CloseWorkflow.cpp
        DropHandler.cpp
        MainWindowContext.cpp
    viewmodels/
      ActorViewModel.cpp
      AnalysisViewModel.cpp
      AnnualViewModel.cpp
      BookingViewModel.cpp
      ContractViewModel.cpp
      ExportViewModel.cpp
      ImportViewModel.cpp
      PropertyViewModel.cpp
      SettingsViewModel.cpp
    workflows/
      AnalysisWorkflow.cpp
      AnnualWorkflow.cpp
      ExportWorkflow.cpp
      ImportWorkflow.cpp
    workspace/
      WorkspaceCommands.cpp
      WorkspacePayloads.cpp
      WorkspaceSelection.cpp
      WorkspaceSelectors.cpp
      WorkspaceStore.cpp
```

## Workspace Rollen

### WorkspaceStore

`WorkspaceStore` besitzt den aktuellen `WorkspaceSnapshot`, die gebundenen
Workspace-Ports fuer Refresh/Lifecycle-Anbindung und die UI-Revision. Er ist
die einzige UI-seitige Quelle fuer geladene Workspace-Daten.

Er darf:

- Snapshots laden und ersetzen.
- `DeletionImpact` auf den gespeicherten Snapshot anwenden.
- die Datenrevision erhoehen und Aenderungssignale emittieren.
- fokussierte Snapshot-Zugriffe anbieten, wenn sie reine Store-Abfragen sind.

Er darf nicht:

- QML-Zeilen bauen.
- Commands validieren oder ausfuehren.
- Feature-Formstate halten.
- Workflow-Use-Cases starten.

### WorkspaceCommands

`WorkspaceCommands` ist der einzige UI-seitige Mutationspfad fuer Workspace-
Zustand. Es mappt UI-nahe Eingaben auf `core/ports/workspace`-Commands und
delegiert an `IWorkspaceWriter`.

Es besitzt:

- Catalog-Commands fuer Actor, Property, Contract, Statement und Transaction.
- Reporting-Commands fuer Analysis und Annual.
- Draft- und Log-Commands, wenn diese persistierter Workspace-Zustand sind.
- Storage-Commands wie new/open/save/save-as.
- Validation-Payload-Mapping fuer QML.

Authoritative Validation bleibt in core. `WorkspaceCommands` darf nur
UI-Payloads fuer Validation-Ergebnisse bauen.

### WorkspaceSelection

`WorkspaceSelection` besitzt die ausgewaehlten IDs ueber Views hinweg:
Actor, Property, Contract, Statement, Transaction, Analysis und Annual.

Selection validiert IDs gegen den aktuellen Store-Zustand und korrigiert
ungueltige Auswahl nach Snapshot-Refresh oder DeletionImpact. Sie fuehrt keine
CRUD-Operationen aus und baut keine Feature-Formulare.

### WorkspaceSelectors

`WorkspaceSelectors` sind reine Read-Model-Projektionen aus `WorkspaceStore`
und optional `WorkspaceSelection`.

Sie bauen:

- `actorRows`
- `propertyRows`
- `contractRows`
- `analysisRows`
- `annualRows`
- `statementRows`
- `statementRowsWithTransactions`
- `statementTransactionRows`
- `propertyTransactionRows`
- `transactionRowById`
- Dropdown- und Lookup-Varianten fuer ViewModels

Selectors mutieren keinen Zustand, rufen keine Writer-Ports auf und enthalten
keine Domain-Regeln. Wenn ein Selector fachliche Regeln braucht, gehoert diese
Regel in core und muss ueber Port/DTO verfuegbar werden.

### WorkspacePayloads

`WorkspacePayloads` enthaelt workspace-spezifische
`QVariantMap`/`QVariantList`-Builder und Validation-Payload-Helfer. Generische
Qt-Konvertierungen bleiben in `ui/presentation/PayloadMapper`.

`WorkspacePayloads` ist kein Sammelort fuer Businesslogik und kein Ersatz fuer
Selectors.

## Bewusst entfernte Workspace-Pfade

Diese Workspace-Dateien sind nicht Teil der Zielarchitektur:

```text
ui/include/ui/workspace/WorkspaceFacade.h
ui/include/ui/workspace/WorkspaceCache.h
ui/include/ui/workspace/WorkspaceCacheModels.h
ui/include/ui/workspace/WorkspaceFilterState.h
ui/include/ui/workspace/WorkspaceRowProjector.h
ui/include/ui/workspace/StatementListModel.h
ui/include/ui/workspace/TransactionListModel.h
ui/include/ui/workspace/TransactionFilterModel.h
ui/include/ui/workspace/AnalysisListModel.h
ui/include/ui/workspace/AnnualListModel.h
ui/include/ui/workspace/IndexedListModel.h
ui/include/ui/workspace/RowListModel.h
ui/include/ui/workspace/WorkflowRunListModel.h

ui/src/workspace/WorkspaceFacade.cpp
ui/src/workspace/WorkspaceFacadeCatalog.cpp
ui/src/workspace/WorkspaceFacadeReporting.cpp
ui/src/workspace/WorkspaceFacadeStorage.cpp
ui/src/workspace/WorkspaceCache.cpp
ui/src/workspace/WorkspaceCacheModels.cpp
ui/src/workspace/WorkspaceFilterState.cpp
ui/src/workspace/WorkspaceRowProjector.cpp
ui/src/workspace/StatementListModel.cpp
ui/src/workspace/TransactionListModel.cpp
ui/src/workspace/TransactionFilterModel.cpp
ui/src/workspace/AnalysisListModel.cpp
ui/src/workspace/AnnualListModel.cpp
```

## Qt ListModel Entscheidung

Workspace-`QAbstractItemModel`-Adapter sind nicht Teil des v0.5-Zielbilds.
Die aktuelle Produktiv-UI konsumiert Workspace-Daten ueber `QVariantList`-
Properties der ViewModels, nicht ueber direkte Workspace-ListModels.

Ein neuer Workspace-ListModel-Typ darf nur eingefuehrt werden, wenn ein
konkreter QML-Consumer `QAbstractItemModel`, Proxy-/Filter-Verhalten oder
Model-Rollenbindung benoetigt. In diesem Fall muss der Consumer dokumentiert
werden, damit kein zweiter stiller Read-Model-Pfad entsteht.

## ViewModels

ViewModels haengen nicht an einem Workspace-Facade-Aggregat. Sie bekommen die
konkreten Workspace-Rollen, die sie brauchen:

- `WorkspaceCommands` fuer Mutationen.
- `WorkspaceSelectors` fuer Zeilen, Dropdowns und Lookups.
- `WorkspaceSelection` fuer ausgewaehlte IDs und Selection-Aktionen.
- `WorkspaceStore` nur fuer Revision oder direkte Snapshot-nahe Read-Anlaesse.

Mehrere Dependencies sind akzeptabel, wenn sie echte Rollen sichtbar machen.
Ein methodenreiches Bundle oder eine neue Facade ist nicht erlaubt.

## Workflows

Workflows orchestrieren Use-Cases, die groesser sind als ein einzelnes
Workspace-CRUD-Command: Import, Export, Analysis und Annual. Sie duerfen
Runner-Ports und Workspace-Rollen konsumieren, bleiben aber hinter den
ViewModels und werden nicht direkt aus QML bedient.

## Adapter

Adapter uebersetzen zwischen Qt/QML-freundlichen Typen und core Use-Case-Ports.
Sie sollen Mapping, Delegation, Cancellation-/Progress-Bridging und Payload-
Konvertierung enthalten. Sobald Adapter fachliche Rows, Summen,
Katalogableitungen oder Exportinhalte ableiten, gehoert diese Logik in core
oder in einen Workspace-Selector, je nachdem ob sie fachlich oder rein
praesentationsnah ist.

## Architektur-Guards

Neue Guard-Tests sollen diese Regeln pruefen:

- UI konsumiert keine non-port-Core-Header.
- Produktions-QML konsumiert Workflows nicht direkt.
- Workflow-Header enthalten keine QML-Properties.
- ViewModels konsumieren keine `WorkspaceFacade`.
- Workspace-ListModels existieren nicht ohne dokumentierten QML-Consumer.
- Workspace-Dateien verwenden keine neuen `Facade`, `Manager`,
  `Projector`- oder Kompatibilitaetsnamen.

Neue Refactor-Schnitte sollen diese Regeln erweitern, nicht umgehen.
