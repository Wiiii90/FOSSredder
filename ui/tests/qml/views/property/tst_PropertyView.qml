/**
 * @file ui/tests/qml/views/property/tst_PropertyView.qml
 * @brief Provides QML tests for PropertyView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "PropertyViewTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property int dataRevision: 0
        property string selectedPropertyId: ""
        property var selectedProperty: null
        property var propertyRowsData: []
        property var contracts: []
        function contractRows() { return contracts || [] }
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
        function indexOfId(rows, id) {
            var list = rows || []
            for (var i = 0; i < list.length; ++i) {
                if (String(list[i].id || "") === String(id || ""))
                    return i
            }
            return -1
        }
        function navigatedId(rows, currentId, delta, fallbackIndex) {
            if (!rows || rows.length === 0)
                return ""
            var index = -1
            for (var i = 0; i < rows.length; ++i) {
                if (String(rows[i].id || "") === String(currentId || "")) {
                    index = i
                    break
                }
            }
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
        function basicFormState(name, aliases, selectedIds) {
            return { name: name || "", aliases: aliases || [], selectedIds: selectedIds || [], aliasInputText: "", aliasIndex: -1 }
        }
    }

    property var propertyViewModel: QtObject {
        property string currentId: ""
        property string name: ""
        property var aliases: []
        property string aliasInputText: ""
        property int aliasIndex: -1
        property var selectedContractIds: []
        property string savedName: ""
        property var savedAliases: []
        property var savedSelectedContractIds: []
        property bool isEdit: false
        property bool hasChanges: false
        property int previousCalls: 0
        property int nextCalls: 0
        property int clearCalls: 0
        property int submitCalls: 0
        property int enterCreateModeCalls: 0
        property int deleteCurrentCalls: 0
        readonly property bool canSubmit: name.trim().length > 0
        readonly property var propertyRows: testCase.session.propertyRowsData
        readonly property var contractRows: testCase.session.contracts
        function canAddAlias(value) { return String(value || "").trim().length > 0 }
        function canRemoveSelectedAlias() { return aliasIndex >= 0 && aliasIndex < aliases.length }
        function isAliasSelected(index) { return aliasIndex === index }
        function isContractSelected(contractId) { return selectedContractIds.indexOf(String(contractId || "").trim()) !== -1 }
        function clear() { clearCalls += 1 }
        function enterCreateMode() { enterCreateModeCalls += 1; testCase.session.selectedPropertyId = ""; isEdit = false }
        function addAlias(value) {}
        function removeAlias(index) {}
        function selectAlias(index) {}
        function requestRemoveSelectedAlias() {}
        function setContractSelected(contractId, selected) {
            const id = String(contractId || "").trim()
            const next = selected ? testCase.session.addUniqueTrimmed(selectedContractIds, id) : testCase.session.removeString(selectedContractIds, id)
            if (next.length === selectedContractIds.length)
                return
            selectedContractIds = next
            hasChanges = true
        }
        function previous() {
            previousCalls += 1
            testCase.session.selectedPropertyId = testCase.session.navigatedId(testCase.session.propertyRowsData, isEdit ? (testCase.session.selectedPropertyId || "") : "", -1, testCase.session.propertyRowsData.length - 1)
        }
        function next() {
            nextCalls += 1
            testCase.session.selectedPropertyId = testCase.session.navigatedId(testCase.session.propertyRowsData, isEdit ? (testCase.session.selectedPropertyId || "") : "", 1, 0)
        }
        function submit() {
            submitCalls += 1
            return "property-new"
        }
        function deleteCurrent() { deleteCurrentCalls += 1 }
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
        id: propertyViewComponent
        PropertyView { width: 960; height: 640; propertyViewModel: testCase.propertyViewModel; theme: testCase.theme }
    }

    function findRequired(root, objectName) {
        var found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function createView() {
        return createTemporaryObject(propertyViewComponent, testCase)
    }

    function init() {
        session.dataRevision = 0
        session.selectedPropertyId = ""
        session.selectedProperty = null
        session.propertyRowsData = [
            { id: "property-1", name: "P1" },
            { id: "property-2", name: "P2" },
            { id: "property-3", name: "P3" }
        ]
        session.contracts = []
    }

    function test_PROP_V_005_navigationStaysEnabledWithSingleRow() {
        session.propertyRowsData = [
            { id: "property-1", name: "P1" }
        ]
        var view = createView()
        var nextButton = findRequired(view, "propertyNextButton")
        var previousButton = findRequired(view, "propertyPreviousButton")

        compare(nextButton.enabled, true)
        compare(previousButton.enabled, true)

        nextButton.clicked()
        compare(session.selectedPropertyId, "property-1")
    }

    function test_PROP_V_003_navigationCyclesThroughCreateMode() {
        session.selectedPropertyId = "property-3"
        session.selectedProperty = { id: "property-3", name: "P3" }
        propertyViewModel.isEdit = true
        var view = createView()

        findRequired(view, "propertyNextButton").clicked()
        compare(session.selectedPropertyId, "")

        findRequired(view, "propertyNextButton").clicked()
        compare(session.selectedPropertyId, "property-1")

        session.selectedPropertyId = "property-1"
        session.selectedProperty = { id: "property-1", name: "P1" }
        propertyViewModel.isEdit = true
        findRequired(view, "propertyPreviousButton").clicked()
        compare(session.selectedPropertyId, "")

        findRequired(view, "propertyPreviousButton").clicked()
        compare(session.selectedPropertyId, "property-3")
    }

    function test_PROP_V_004_navigationStartsAtEdgesFromCreateMode() {
        var view = createView()

        findRequired(view, "propertyNextButton").clicked()
        compare(session.selectedPropertyId, "property-1")

        session.selectedPropertyId = ""
        propertyViewModel.isEdit = false
        findRequired(view, "propertyPreviousButton").clicked()
        compare(session.selectedPropertyId, "property-3")
    }

    function test_PROP_V_006_contractSelectionEnablesUpdateButtonInEditMode() {
        session.contracts = [
            { id: "contract-1", name: "Lease" }
        ]
        session.selectedPropertyId = "property-2"
        session.selectedProperty = { id: "property-2", name: "P2" }
        propertyViewModel.isEdit = true
        propertyViewModel.name = "P2"
        propertyViewModel.hasChanges = false

        var view = createView()
        var checkBox = findRequired(view, "propertyContractCheckBox")
        var updateButton = findRequired(view, "propertyUpdateButton")

        compare(updateButton.enabled, false)
        checkBox.checked = true
        checkBox.toggled()

        compare(propertyViewModel.hasChanges, true)
        compare(updateButton.enabled, true)
    }
}
