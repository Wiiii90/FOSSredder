# Analyse: UI-Modelle und UI-Controller

## Scope
Diese Analyse betrachtet nur den UI-C++-Bereich rund um
- `ui/include/ui/models`
- `ui/src/models`
- `ui/include/ui/controllers`
- `ui/src/controllers`

Nicht enthalten sind bewusst:
- QML-Dateien
- Core-Implementierung
- Persistence
- Services
- die bereits vorhandene Zwischenanalyse `UI_SRC_MODEL_PREANALYSIS.md`

## Kurzfazit
Die Architektur im UI-Bereich ist grundsätzlich erkennbar, aber nicht überall sauber getrennt:
- `ui/controllers` sind die eigentlichen Orchestrierer zwischen QML und Core.
- `ui/models` sind teils echte QML-Listenmodelle, teils Workflow-/Editor-Zustände.
- Einige Modelle sind sehr sauber als Adapter umgesetzt.
- Einige Workflow-Klassen tragen zu viel Logik und vermischen State, Mapping und fachnahe Ableitung.

Der wichtigste Grundsatz ist bereits vorhanden: Businesslogik liegt größtenteils im Core, die UI orchestriert und projiziert. Die Schwachstellen sind eher Granularität, Namensgebung und Verantwortlichkeitsvermischung innerhalb der UI-Schicht.

## Architektur der UI-Schicht
Die UI-Schicht folgt im Kern diesem Muster:

1. **Bootstrap / AppContext**
   - stellt QML-exponierte Objekte bereit
   - verdrahtet Controller, State und Hilfsobjekte

2. **Controller**
   - nehmen QML-Inputs entgegen
   - rufen Core-Use-Cases auf
   - liefern QML-freundliche Payloads zurück
   - sollten möglichst dünn sein

3. **Modelle**
   - bilden Core-Entities oder UI-Workflow-State auf QML-Rollen ab
   - liefern Role Names, QVariantMap, QVariantList oder QAbstractListModel-Verhalten

4. **State / Projection**
   - hält UI-Zustand
   - normalisiert Payloads
   - projiziert Core-State in QML-fähige Strukturen

Das ist eine legitime Architektur. Sie wirkt nur an mehreren Stellen gewachsen und nicht streng genug getrennt.

## UI-Modelle im Einzelnen

### 1. `ActorList`
**Typ:** direktes Listenmodell für `core::domain::Actor`

**Bewertung:** sauber und passend.
- klares Mapping zu Core
- Rollen sind nachvollziehbar
- bietet die nötigen Add-/Remove-Operationen

**Fazit:** behalten.

### 2. `PropertyList`
**Typ:** direktes Listenmodell für `core::domain::Property`

**Bewertung:** sauber und passend.
- einfache Rollen
- keine auffällige Vermischung

**Fazit:** behalten.

### 3. `ContractList`
**Typ:** direktes Listenmodell für `core::domain::Contract`

**Bewertung:** grundsätzlich sauber.
- aber schon leicht adapterlastig durch Array-/Listen-Konvertierung
- die `get(int)`-Methode ist eher Convenience als reine Model-API

**Fazit:** behalten, aber auf Redundanz prüfen.

### 4. `StatementList`
**Typ:** direktes Listenmodell für `core::domain::Statement`

**Bewertung:** sehr sauber.

**Fazit:** behalten.

### 5. `AnnualList`
**Typ:** direktes Listenmodell für `core::domain::Annual`

**Bewertung:** sauber.
- geringe Komplexität
- gute Entsprechung zum Core

**Fazit:** behalten.

### 6. `AnalysisList`
**Typ:** Listenmodell für `core::domain::Analysis`, aber mit Cache-/Serialisierungslogik

**Bewertung:** wichtigster Sonderfall unter den Standardlisten.
- enthält Cache für `adjustmentsJsonById_`
- serialisiert Core-Daten selbst
- ruft `AnalysisRequestComposer` auf
- macht mehr als reine Anzeige

**Architekturfrage:**
Hier liegt bereits UI-spezifische Projektion plus Komfortlogik.
Das ist nicht automatisch falsch, aber es ist der erste Kandidat für Aufspaltung in:
- reines Listmodell
- dedizierten Adapter/Mapper für `adjustmentsJson`

**Fazit:** funktional okay, strukturell der erste echte Refactor-Kandidat.

### 7. `TransactionList`
**Typ:** Listenmodell für `core::domain::Transaction`, aber mit Zusatzprojektion

**Bewertung:** deutlich komplexer als die anderen Standardlisten.
- enthält Rollen für Anzeige
- hält `contractTypeById_`
- baut `QVariantMap`-Payloads
- hat `get(int)`-Convenience
- rebuildet ID-Index selbst

**Architekturfrage:**
Das Modell ist gleichzeitig
- View-Adapter
- Projection-Helper
- teilweise Payload-Builder

Das ist der Bereich, in dem die Schichtung am wenigsten klar ist.

**Fazit:** fachlich korrekt, aber zu breit. Starker Kandidat für Aufteilung.

### 8. `ImportRunList`
**Typ:** UI-Run-Log-Modell, kein Core-Entity-Mirror

**Bewertung:** sinnvoll als UI-Adapter.
- dient Logging / Historie
- flach und gut für QML
- nicht 1:1 Core-Entity

**Fazit:** behalten, aber Rollen nur so weit wie nötig.

### 9. `ExportRunList`
**Typ:** UI-Run-Log-Modell, kein Core-Entity-Mirror

**Bewertung:** wie `ImportRunList` sinnvoll.
- UI benötigt eine eigene flache Darstellung

**Fazit:** behalten, aber Payload-/Rollen-Umfang prüfen.

### 10. `TransactionDraft`
**Typ:** UI-Workflow-Datenobjekt für einen Draft-Row

**Bewertung:** wichtig und sinnvoll.
- keine Core-Entity, sondern Editor-/Workflow-State
- entspricht einem UI-Formularzustand
- enthält Vorschläge, Auswahlzustand und UI-nahe Flags

**Namensbewertung:**
Der Name ist verständlich, aber semantisch etwas zu nah am Core-Draft.
Wichtig ist vor allem, dass er **nicht** mit einer Listenklasse verwechselt wird.

**Fazit:** behalten, aber Namens- und Verantwortlichkeitsdisziplin beachten.

### 11. `TransactionDraftList`
**Typ:** interaktives Listenmodell für Draft-Rows

**Bewertung:** passend, aber komplex.
- echte QML-Liste
- bearbeitbar
- viele Setter
- viele Rollen
- arbeitet als Editor-Modell

**Fazit:** behalten, weil QML das braucht. Aber Rollen und Setter auf tatsächlichen Bedarf prüfen.

### 12. `StatementDraft`
**Typ:** Workflow-/Editor-Container für einen Statement-Draft

**Bewertung:** funktional sinnvoll, aber semantisch kein klassisches „Model“.
- hält Name, DraftId, Index, Catalog-State
- enthält `TransactionDraftList`
- bietet Navigation und Refresh
- orchestriert einen interaktiven Draft-Workflow

**Architekturfrage:**
Das ist eher ein `DraftWorkflow` oder `DraftEditorState` als ein reines Model.
Die Klasse ist okay, aber der Name suggeriert zu wenig, dass es sich um einen Workflow-Container handelt.

**Fazit:** behalten, aber die Rolle als Workflow-State klarer machen.

### 13. `TransactionFilter`
**Typ:** UI-Filter-/Proxy-State

**Bewertung:** klarer Spezialfall.
- kein Domain-Objekt
- kein klassisches Modell
- QSortFilterProxyModel mit Filterzustand

**Fazit:** sinnvoll, aber eher „UI state / proxy“ als „Model“.

### 14. `ImportSuggestion`
**Typ:** UI-spezifisches Hilfsmodell für Import-Matching

**Bewertung:** gut als Hilfstyp.
- kein klassisches Model im Sinne einer Liste
- mehr Datenstruktur als UI-Model

**Fazit:** sinnvoll, aber inhaltlich eher Support-Typ.

## Namensgebung der Modelle

### Positiv
- `ActorList`, `PropertyList`, `ContractList`, `StatementList`, `AnnualList`, `AnalysisList` sind konsistent.
- `TransactionList` ist verständlich.
- `ImportRunList` und `ExportRunList` sind logisch.

### Auffällig
- `StatementDraft` ist semantisch eher ein Workflow-Container als ein einzelner Draft.
- `TransactionDraft` ist als Name okay, aber sollte nicht als Listenklasse erscheinen.
- `TransactionFilter` ist funktional richtig, aber vom Namen her eher ein Spezial-Proxy als ein Model.

### Wichtig
Die Modelle sind **nicht** einfach 1:1 zum Core gespiegelt. Das muss auch nicht sein.
Aber sie sollten so nah wie sinnvoll am Core bleiben und nur UI-spezifische Erweiterungen besitzen.

## UI-Controller im Einzelnen

### 1. `ActorController`
**Bewertung:** dünner und sauberer Controller.
- liest Core-State
- gibt Payloads zurück
- delegiert Mutationen an Core

**Fazit:** gut.

### 2. `PropertyController`
**Bewertung:** ebenso sauber.

**Fazit:** gut.

### 3. `ContractController`
**Bewertung:** sauber, aber mit Hilfslogik für Payload-Rückgabe.

**Fazit:** gut.

### 4. `StatementController`
**Bewertung:** sauber.
- kleine Zusatzlogik beim Lookup
- ansonsten klassischer Adapter

**Fazit:** gut.

### 5. `TransactionController`
**Bewertung:** deutlich schwerer als die anderen CRUD-Controller.
- arbeitet mit Draft-Normalisierung
- validiert Statuswerte
- erzeugt Transaktionen aus UI-Drafts
- enthält also Orchestrierungslogik plus Datenanpassung

**Fazit:** funktional korrekt, aber schon eher Workflow-Adapter als „thin controller“.

### 6. `AnalysisController`
**Bewertung:** einer der komplexeren Controller.
- baut Analysis-Configs
- baut Filter-Specs
- baut Adjustments-JSON
- compute() mit Snapshot + Service
- preview() mit eigener Ableitungslogik

**Architekturfrage:**
Hier ist der Controller nicht mehr nur dünner Adapter, sondern auch Input-/Payload-Orchestrator.
Das ist okay, aber diese Klasse ist ein klarer Kandidat für zukünftige Aufspaltung.

**Fazit:** wichtig, aber zu breit.

### 7. `ImportController`
**Bewertung:** größter Workflow-Controller.
- Job-Start und Cancel
- Run-Log-Handling
- Draft-Open/Restore
- State Snapshot
- Error Reporting
- Queue-Handling

**Architekturfrage:**
Das ist kein einfacher Controller mehr, sondern ein kompletter Workflow-Orchestrierer.

**Fazit:** fachlich sinnvoll, aber strukturell sehr breit. Klarer Refactor-Kandidat.

### 8. `ExportController`
**Bewertung:** ebenfalls großer Workflow-Controller.
- async export
- snapshot handling
- run log persistence
- progress / cancel / pause
- payload transport

**Fazit:** sinnvoll, aber zu breit und zu zuständigkeitslastig.

### 9. `DraftController`
**Bewertung:** kombiniert Draft-Finalisierung mit Ableitungs- und UI-Synchronisationslogik.
- baut Finalisierungsinput
- synchronisiert aktuelle Draft-Auswahl
- reichert Auswahl über Core-Ableitungen an
- verarbeitet Amount Parsing

**Fazit:** ebenfalls breit und workflow-lastig.

### 10. `StorageController`
**Bewertung:** relativ sauber.
- delegiert Dateioperationen
- Fehlerbehandlung ist klar

**Fazit:** gut.

### 11. `LanguageController`
**Bewertung:** UI-infrastrukturell notwendig, aber nicht fachlich domänennah.
- gut gekapselt
- zuständig für Runtime-Language-Handling

**Fazit:** okay.

### 12. `SettingsController`
**Bewertung:** eher stateful settings facade als klassischer Domain-Controller.
- verwaltet Persistenzwerte
- hält viele UI-Optionen

**Fazit:** inhaltlich okay, aber als zentraler Settings-Container sehr breit.

### 13. `FileSystemController`
**Bewertung:** klein und klar.

**Fazit:** gut.

## Was ist die UI-Architektur hier wirklich?
Kurz und präzise:

- **`ui/bootstrap`** = Verdrahtung und Registrierung für QML
- **`ui/controllers`** = QML-Entry-Points, die Core-Use-Cases anstoßen
- **`ui/models`** = QML-Modelle und UI-Workflow-States
- **`ui/state`** = Session-, Selection- und Projection-Mechanik
- **`ui/import`, `ui/export`, `ui/analysis`** = Workflow-spezifische Adapter und Mapper
- **`ui/payload`** = Key- und Payload-Kontrakte für QML
- **`ui/util`** = Hilfen für Core-Guards, String-Konvertierung etc.

Das ist keine chaotische Architektur, aber sie ist **zu breit gewachsen** und nicht konsequent nach Verantwortlichkeiten sortiert.

## Wichtigste strukturelle Baustellen

### Hoch priorisiert
1. `TransactionList` entflechten: Anzeige, Projection und Payload nicht in einer Klasse vermischen.
2. `AnalysisList` prüfen: Cache-/Serialisierungslogik evtl. auslagern.
3. `ImportController` und `ExportController` als Workflows klarer isolieren.
4. `DraftController` auf dünnere Orchestrierung reduzieren.
5. `StatementDraft` als Workflow-Container klarer benennen bzw. abgrenzen.

### Mittel priorisiert
1. Rollen in Listenmodellen auf tatsächliche Nutzung prüfen.
2. `TransactionFilter` als Spezial-Proxy statt generisches Modell betrachten.
3. `SettingsController` eventuell in klarere Settings-Unterblöcke teilen.

### Niedrig priorisiert
1. Namensfeinschliff bei Workflow-Objekten.
2. Doppelte / alte Convenience-APIs im `get(int)`-Stil prüfen.
3. Kleine Bereinigungen an Kommentar- und Dateibenennungen.

## Antwort auf die Kernfrage: Sind die Modelle paritätisch zum Core?
**Teilweise ja, aber nicht strikt.**

Das ist aus meiner Sicht auch richtig so:
- Core ist die Domäne.
- UI-Modelle sind Präsentations- und Workflow-Adapter.
- Sie müssen nicht 1:1 paritätisch sein.

Wichtig ist nur:
- direkte Listenmodelle sollten nah am Core bleiben
- Workflow-Modelle dürfen UI-spezifisch sein
- die Vermischung von Anzeige, Cache, Ablaufsteuerung und Fachableitung sollte möglichst reduziert werden

## Schlussfolgerung
Die UI-Schicht ist nicht „kaputt“, aber sie ist in Teilen zu breit und zu wenig klar aufgeteilt.

Die saubersten Bereiche sind:
- die direkten Entity-Listenmodelle
- die kleinen CRUD-Controller
- die Bootstrap-/Kontextverdrahtung

Die kritischsten Bereiche sind:
- `TransactionList`
- `AnalysisList`
- `ImportController`
- `ExportController`
- `DraftController`
- `StatementDraft`
- `TransactionDraftList`

Wenn der nächste Schritt kommt, sollte er **nicht** gleich ein großer Umbau sein, sondern eine gezielte Entflechtung der Workflow- und Projection-Bereiche.
