/**
 * @file ui/tests/qml/views/contract/tst_ContractView.qml
 * @brief Provides QML tests for ContractView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ContractViewTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property int dataRevision: 0
        property string selectedContractId: ""
        property var selectedContract: null
        property var contractRowsData: []
        property var actors: []
        property var properties: []
        function contractRows() { return contractRowsData || [] }
        function actorRows() { return actors || [] }
        function displayRowsWithEmpty(rows, emptyLabel, textKey) {
            var out = [{ id: "", display: emptyLabel }]
            var list = rows || []
            for (var i = 0; i < list.length; ++i) {
                out.push({
                    id: String(list[i].id || ""),
                    display: String(list[i][textKey] || "")
                })
            }
            return out
        }
        function indexOfId(rows, id) {
            var list = rows || []
            for (var i = 0; i < list.length; ++i) {
                if (String(list[i].id || "") === String(id || ""))
                    return i
            }
            return -1
        }
        function addUniqueTrimmed(values, value) {
            var out = values ? values.slice(0) : []
            var next = String(value || "").trim()
            if (next.length === 0 || out.indexOf(next) !== -1)
                return out
            out.push(next)
            return out
        }
        function removeString(values, value) {
            var out = values ? values.slice(0) : []
            var target = String(value || "")
            var index = out.indexOf(target)
            if (index < 0)
                return out
            out.splice(index, 1)
            return out
        }
        function removeAt(values, index) {
            var out = values ? values.slice(0) : []
            if (index < 0 || index >= out.length)
                return out
            out.splice(index, 1)
            return out
        }
        function navigatedId(rows, currentId, delta, fallbackIndex) {
            if (!rows || rows.length === 0)
                return ""
            var index = indexOfId(rows, currentId)
            if (index < 0) {
                index = delta > 0 ? 0 : (delta < 0 ? rows.length - 1 : fallbackIndex)
                return String(rows[index].id || "")
            }
            if (delta > 0)
                return index >= rows.length - 1 ? "" : String(rows[index + 1].id || "")
            if (delta < 0)
                return index <= 0 ? "" : String(rows[index - 1].id || "")
            return String(rows[index].id || "")
        }
        function contractFormState(name, type, actorIds, propertyIds, aliases) {
            return { name: name || "", type: type || "", selectedActorIds: actorIds || [], selectedPropertyIds: propertyIds || [], aliases: aliases || [], aliasInputText: "", aliasIndex: -1 }
        }
        function createContractObject(source) {
            return TestSupport.createContractObject(testCase, source)
        }
    }

    property var contractViewModel: QtObject {
        readonly property string currentId: testCase.session.selectedContractId
        property string name: ""
        property string type: ""
        property string allocatableMode: "mixed"
        property var aliases: []
        property string aliasInputText: ""
        property int aliasIndex: -1
        property var selectedActorIds: []
        property var selectedPropertyIds: []
        readonly property bool isEdit: testCase.session.selectedContractId.length > 0
        property bool hasChanges: false
        readonly property bool canSubmit: name.trim().length > 0
                                          && type.trim().length > 0
                                          && (selectedActorIds.length > 0 || selectedPropertyIds.length > 0)
        readonly property var contractRows: testCase.session.contractRows()
        readonly property var actorRows: testCase.session.actorRows()
        readonly property var actorDisplayRows: testCase.session.displayRowsWithEmpty(actorRows, "No actor", "name")
        readonly property var propertyRows: testCase.session.properties
        readonly property int selectedActorIndex: {
            const id = selectedActorIds.length > 0 ? selectedActorIds[0] : ""
            const index = testCase.session.indexOfId(actorDisplayRows, id)
            return index >= 0 ? index : 0
        }

        function canAddAlias(value) { return String(value || "").trim().length > 0 }
        function canRemoveSelectedAlias() { return aliasIndex >= 0 && aliasIndex < aliases.length }
        function isAliasSelected(index) { return aliasIndex === index }
        function addAlias(value) {
            aliases = testCase.session.addUniqueTrimmed(aliases, value)
            aliasIndex = aliases.length > 0 ? aliases.length - 1 : -1
            aliasInputText = ""
            hasChanges = true
        }
        function removeAlias(index) {
            aliases = testCase.session.removeAt(aliases, index)
            aliasIndex = aliases.length > 0 ? Math.min(index, aliases.length - 1) : -1
            hasChanges = true
        }
        function requestRemoveSelectedAlias() { if (canRemoveSelectedAlias()) removeAlias(aliasIndex) }
        function selectPrimaryActor(actorId) {
            const id = String(actorId || "").trim()
            selectedActorIds = id.length > 0 ? [id] : []
            hasChanges = true
        }
        function setPropertySelected(propertyId, selected) {
            selectedPropertyIds = selected
                    ? testCase.session.addUniqueTrimmed(selectedPropertyIds, propertyId)
                    : testCase.session.removeString(selectedPropertyIds, propertyId)
            hasChanges = true
        }
        function clear() {
            name = ""
            type = ""
            aliases = []
            aliasInputText = ""
            aliasIndex = -1
            selectedActorIds = []
            selectedPropertyIds = []
            hasChanges = false
        }
        function enterCreateMode() {
            testCase.session.selectedContractId = ""
            testCase.session.selectedContract = null
            clear()
        }
        function selectContract(contractId) {
            testCase.session.selectedContractId = String(contractId || "")
            syncSelectionObject()
        }
        function previous() {
            const rows = testCase.session.contractRows()
            testCase.session.selectedContractId = testCase.session.navigatedId(rows, isEdit ? testCase.session.selectedContractId : "", -1, rows.length - 1)
            syncSelectionObject()
        }
        function next() {
            const rows = testCase.session.contractRows()
            testCase.session.selectedContractId = testCase.session.navigatedId(rows, isEdit ? testCase.session.selectedContractId : "", 1, 0)
            syncSelectionObject()
        }
        function submit() { return isEdit ? testCase.session.selectedContractId : "contract-new" }
        function deleteCurrent() { enterCreateMode() }
        function syncSelectionObject() {
            const id = testCase.session.selectedContractId || ""
            if (id.length === 0) {
                testCase.session.selectedContract = null
                clear()
                return
            }
            const rows = testCase.session.contractRows()
            for (var i = 0; i < rows.length; ++i) {
                if (String(rows[i].id || "") === id) {
                    testCase.session.selectedContract = testCase.session.createContractObject(rows[i])
                    name = String(rows[i].name || "")
                    type = String(rows[i].type || "")
                    hasChanges = false
                    return
                }
            }
        }
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int viewFormSpacing: 8
        property int formLabelWidth: 120
        property int spacing: 8
        property int spacingSmall: 6
        property int spacingLarge: 20
        property int controlHeight: 32
        property int formFieldWidth: 220
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
        id: contractViewComponent
        ContractView { width: 960; height: 640; contractViewModel: testCase.contractViewModel; theme: testCase.theme }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createView() {
        return createTemporaryObject(contractViewComponent, testCase)
    }

    function init() {
        session.dataRevision = 0
        session.selectedContractId = ""
        session.selectedContract = null
        session.contractRowsData = [
            { id: "contract-1", name: "C1" },
            { id: "contract-2", name: "C2" },
            { id: "contract-3", name: "C3" }
        ]
        session.actors = []
        session.properties = []
        contractViewModel.enterCreateMode()
    }

    function test_CON_V_005_navigationStaysEnabledWithSingleRow() {
        session.contractRowsData = [
            { id: "contract-1", name: "C1" }
        ]
        var view = createView()
        var nextButton = findRequired(view, "contractNextButton")
        var previousButton = findRequired(view, "contractPreviousButton")

        compare(nextButton.enabled, true)
        compare(previousButton.enabled, true)

        nextButton.clicked()
        compare(session.selectedContractId, "contract-1")
    }

    function test_CON_V_003_navigationCyclesThroughCreateMode() {
        session.selectedContractId = "contract-3"
        session.selectedContract = session.createContractObject({ id: "contract-3", name: "C3" })
        var view = createView()

        findRequired(view, "contractNextButton").clicked()
        compare(session.selectedContractId, "")

        findRequired(view, "contractNextButton").clicked()
        compare(session.selectedContractId, "contract-1")

        session.selectedContractId = "contract-1"
        session.selectedContract = session.createContractObject({ id: "contract-1", name: "C1" })
        findRequired(view, "contractPreviousButton").clicked()
        compare(session.selectedContractId, "")

        findRequired(view, "contractPreviousButton").clicked()
        compare(session.selectedContractId, "contract-3")
    }

    function test_CON_V_004_createModeNavigationStartsAtEdges() {
        var view = createView()

        findRequired(view, "contractNextButton").clicked()
        compare(session.selectedContractId, "contract-1")

        session.selectedContractId = ""
        findRequired(view, "contractPreviousButton").clicked()
        compare(session.selectedContractId, "contract-3")
    }
}
