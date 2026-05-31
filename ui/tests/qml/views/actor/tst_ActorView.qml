/**
 * @file ui/tests/qml/views/actor/tst_ActorView.qml
 * @brief Provides QML tests for ActorView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ActorViewTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property int dataRevision: 0
        property string selectedActorId: ""
        property var selectedActor: null
        property var actorRowsData: []
        property var contracts: []
        property var actorRows: actorRowsData
        property var contractRows: contracts
        onSelectedActorIdChanged: {
            if (selectedActorId.length === 0) {
                selectedActor = null
                if (testCase.actorState)
                    testCase.actorState.syncFromSelection(false)
                return
            }
            if (selectedActor && String(selectedActor.id || "") === selectedActorId) {
                if (testCase.actorState)
                    testCase.actorState.syncFromSelection(false)
                return
            }

            const list = actorRowsData || []
            for (let i = 0; i < list.length; ++i) {
                const row = list[i] || ({})
                if (String(row.id || "") === selectedActorId) {
                    selectedActor = {
                        id: String(row.id || ""),
                        name: String(row.name || ""),
                        aliases: row.aliases || [],
                        contractIds: row.contractIds || []
                    }
                    if (testCase.actorState)
                        testCase.actorState.syncFromSelection(false)
                    return
                }
            }

            selectedActor = {
                id: selectedActorId,
                name: "",
                aliases: [],
                contractIds: []
            }
            if (testCase.actorState)
                testCase.actorState.syncFromSelection(false)
        }
        onSelectedActorChanged: if (testCase.actorState) testCase.actorState.syncFromSelection(true)
        function addUniqueTrimmed(values, value) {
            const out = values ? values.slice(0) : []
            const next = String(value || "").trim()
            if (next.length === 0 || out.indexOf(next) !== -1)
                return out
            out.push(next)
            return out
        }
        function removeAt(values, index) {
            const out = values ? values.slice(0) : []
            if (index < 0 || index >= out.length)
                return out
            out.splice(index, 1)
            return out
        }
        function removeString(values, value) {
            const out = values ? values.slice(0) : []
            const target = String(value || "")
            for (let i = out.length - 1; i >= 0; --i) {
                if (String(out[i] || "") === target)
                    out.splice(i, 1)
            }
            return out
        }
        function normalizedStringListKey(values) {
            const list = values ? values.slice(0) : []
            list.sort()
            return JSON.stringify(list.map(function(item) { return String(item || "") }))
        }
        function formStateChanged(savedName, savedAliases, savedContractIds, currentName, aliases, contractIds) {
            return String(savedName || "") !== String(currentName || "")
                    || normalizedStringListKey(savedAliases) !== normalizedStringListKey(aliases)
                    || normalizedStringListKey(savedContractIds) !== normalizedStringListKey(contractIds)
        }
        function basicFormState(name, aliases, selectedIds) {
            return { name: name || "", aliases: aliases || [], selectedIds: selectedIds || [], aliasInputText: "", aliasIndex: -1 }
        }
        function deleteNextSelectionId(rows, removedId, fallbackIndex, key) {
            if (!rows || rows.length === 0)
                return ""
            const kept = []
            for (let i = 0; i < rows.length; ++i) {
                const rowId = String(rows[i][key] || "")
                if (rowId !== String(removedId || ""))
                    kept.push(rows[i])
            }
            if (kept.length === 0)
                return ""
            const idx = Math.max(0, Math.min(fallbackIndex, kept.length - 1))
            return String(kept[idx][key] || "")
        }
        function navigatedSelectionId(rows, currentId, delta, fallbackIndex, key) {
            const list = rows || []
            if (list.length === 0)
                return ""
            const idKey = key || "id"
            let currentIndex = -1
            for (let i = 0; i < list.length; ++i) {
                if (String(list[i][idKey] || "") === String(currentId || "")) {
                    currentIndex = i
                    break
                }
            }
            if (currentIndex < 0) {
                if (delta > 0)
                    return String(list[0][idKey] || "")
                if (delta < 0)
                    return String(list[list.length - 1][idKey] || "")
                const fallback = Math.max(0, Math.min(fallbackIndex, list.length - 1))
                return String(list[fallback][idKey] || "")
            }
            if ((delta > 0 && currentIndex === list.length - 1)
                    || (delta < 0 && currentIndex === 0))
                return ""
            const nextIndex = currentIndex + delta
            if (nextIndex < 0 || nextIndex >= list.length)
                return ""
            return String(list[nextIndex][idKey] || "")
        }
    }

    property var actorState: QtObject {
        property string currentOwnerId: ""
        property string name: ""
        property var aliases: []
        property string aliasInputText: ""
        property int aliasIndex: -1
        property var selectedContractIds: []
        property string savedName: ""
        property var savedAliases: []
        property var savedSelectedContractIds: []
        readonly property bool isEdit: testCase.session.selectedActorId.length > 0
        readonly property bool hasChanges: !isEdit
                ? name.trim().length > 0
                : testCase.session.formStateChanged(savedName,
                                                    savedAliases,
                                                    savedSelectedContractIds,
                                                    name,
                                                    aliases,
                                                    selectedContractIds)
        readonly property bool canSubmit: name.trim().length > 0
        readonly property var actorRows: testCase.session.actorRows
        readonly property var contractRows: testCase.session.contractRows

        function canAddAlias(value) {
            return String(value || "").trim().length > 0
        }

        function canRemoveSelectedAlias() {
            return aliasIndex >= 0 && aliasIndex < aliases.length
        }

        function isAliasSelected(index) {
            return aliasIndex === index
        }

        function isContractSelected(contractId) {
            return selectedContractIds.indexOf(String(contractId || "").trim()) !== -1
        }

        function addAlias(value) {
            const next = testCase.session.addUniqueTrimmed(aliases || [], value || "")
            if (next.length === aliases.length)
                return
            aliases = next
            aliasIndex = next.length - 1
            aliasInputText = ""
        }

        function removeAlias(index) {
            const next = testCase.session.removeAt(aliases || [], index)
            if (next.length === aliases.length)
                return
            aliases = next
            aliasIndex = next.length > 0 ? Math.min(index, next.length - 1) : -1
        }

        function selectAlias(index) {
            aliasIndex = index
        }

        function requestRemoveSelectedAlias() {
            if (!canRemoveSelectedAlias())
                return
            removeAlias(aliasIndex)
        }

        function setContractSelected(contractId, selected) {
            const next = selected
                    ? testCase.session.addUniqueTrimmed(selectedContractIds || [], contractId)
                    : testCase.session.removeString(selectedContractIds || [], contractId)
            if (JSON.stringify(next) === JSON.stringify(selectedContractIds))
                return
            selectedContractIds = next
        }

        function clearFormState() {
            name = ""
            aliases = []
            aliasInputText = ""
            aliasIndex = -1
            selectedContractIds = []
        }

        function captureSavedState() {
            savedName = name
            savedAliases = aliases.slice(0)
            savedSelectedContractIds = selectedContractIds.slice(0)
        }

        function syncFromSelection(forceReload) {
            const currentId = testCase.session.selectedActorId || ""
            if (!forceReload && currentOwnerId === currentId)
                return
            currentOwnerId = currentId
            if (currentId.length === 0 || !testCase.session.selectedActor) {
                clearFormState()
                captureSavedState()
                return
            }
            const current = testCase.session.selectedActor
            const state = testCase.session.basicFormState(current.name || "",
                                                          current.aliases || [],
                                                          current.contractIds || [])
            name = state.name || ""
            aliases = state.aliases || []
            aliasInputText = state.aliasInputText || ""
            aliasIndex = state.aliasIndex !== undefined ? state.aliasIndex : -1
            selectedContractIds = state.selectedIds || []
            captureSavedState()
        }

        function previous() {
            const rows = testCase.session.actorRows || []
            if (rows.length === 0)
                return
            const nextId = testCase.session.navigatedSelectionId(rows,
                                                                  isEdit ? (testCase.session.selectedActorId || "") : "",
                                                                  -1,
                                                                  rows.length - 1)
            testCase.session.selectedActorId = nextId
        }

        function next() {
            const rows = testCase.session.actorRows || []
            if (rows.length === 0)
                return
            const nextId = testCase.session.navigatedSelectionId(rows,
                                                                  isEdit ? (testCase.session.selectedActorId || "") : "",
                                                                  1,
                                                                  0)
            testCase.session.selectedActorId = nextId
        }
    }

    property var workspaceFacade: QtObject {
        property var actorRows: testCase.session.actorRows
        property var contractRows: testCase.session.contractRows
        property var actorState: testCase.actorState
        function saveActor(id, name, aliases, contractIds) { return id || "actor-new" }
        function deleteActor(id) {}
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int viewFormSpacing: 8
        property int formLabelWidth: 120
        property int spacing: 8
        property int spacingSmall: 6
        property int spacingLarge: 20
        property int viewCompactActionButtonSize: 28
        property int viewActionButtonWidth: 120
        property int viewNavigationButtonWidth: 40
        property int viewAliasGroupSpacing: 6
        property int viewAliasPanelMinHeight: 60
        property int viewAliasPanelPreferredHeight: 90
        property int viewAliasChipHeight: 24
        property int viewAliasChipRadius: 4
        property int viewSelectionPanelMinHeight: 80
        property int viewSelectionPanelPreferredHeight: 120
        property int radius: 3
        property int borderWidthThin: 1
        property color selectionHighlight: "#aaccee"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color borderSoft: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: actorViewComponent
        ActorView { width: 960; height: 640; actorState: testCase.actorState; theme: testCase.theme }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createView() {
        return createTemporaryObject(actorViewComponent, testCase)
    }

    function init() {
        session.dataRevision = 0
        session.selectedActorId = ""
        session.selectedActor = null
        session.actorRowsData = [
            { id: "actor-1", name: "A1" },
            { id: "actor-2", name: "A2" },
            { id: "actor-3", name: "A3" }
        ]
        session.contracts = []
        if (testCase.actorState)
            testCase.actorState.syncFromSelection(true)
    }

    function test_ACT_V_005_navigationStaysEnabledWithSingleRow() {
        session.actorRowsData = [
            { id: "actor-1", name: "A1" }
        ]
        var view = createView()
        var nextButton = findRequired(view, "actorNextButton")
        var previousButton = findRequired(view, "actorPreviousButton")

        compare(nextButton.enabled, true)
        compare(previousButton.enabled, true)

        nextButton.clicked()
        compare(session.selectedActorId, "actor-1")
    }

    function test_ACT_V_003_navigationCyclesThroughCreateMode() {
        session.selectedActorId = "actor-3"
        session.selectedActor = { id: "actor-3", name: "A3" }
        var view = createView()

        findRequired(view, "actorNextButton").clicked()
        compare(session.selectedActorId, "")

        findRequired(view, "actorNextButton").clicked()
        compare(session.selectedActorId, "actor-1")

        session.selectedActorId = "actor-1"
        session.selectedActor = { id: "actor-1", name: "A1" }
        findRequired(view, "actorPreviousButton").clicked()
        compare(session.selectedActorId, "")

        findRequired(view, "actorPreviousButton").clicked()
        compare(session.selectedActorId, "actor-3")
    }

    function test_ACT_V_004_createModeNavigationStartsAtEdges() {
        var view = createView()

        findRequired(view, "actorNextButton").clicked()
        compare(session.selectedActorId, "actor-1")

        session.selectedActorId = ""
        findRequired(view, "actorPreviousButton").clicked()
        compare(session.selectedActorId, "actor-3")
    }
}
