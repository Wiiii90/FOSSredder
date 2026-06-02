/**
 * @file ui/tests/qml/views/import/tst_TransactionDraftContractPropertyPanel.qml
 * @brief Provides QML tests for TransactionDraftContractPropertyPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "TransactionDraftContractPropertyPanelTests"
    when: windowShown
    width: 900
    height: 320

    property var theme: QtObject {
        property int spacingSmall: 6
        property int radius: 3
        property color border: "#cccccc"
        property color surfaceAlt: "#f5f5f5"
        property color textMuted: "#666666"
        property color danger: "#aa0000"
        property color successStrong: "#008800"
        property color warning: "#aa8800"
    }

    property var transactionViewModel: QtObject {
        property var propertyOptions: [{ id: "property-1", display: "Energy" }, { id: "property-2", display: "Office" }]
        property string propertyName: ""
        property bool canAddProperty: true
        property int addPropertyCalls: 0
        property int selectedPropertyCalls: 0
        property string lastPropertyId: ""
        property bool lastPropertySelected: false
        property real propertySuggestionConfidence: 0.8
        property string propertySuggestionSummary: "Property suggestion"
        function suggestionTone(confidence) { return confidence >= 0.75 ? 2 : 0 }
        function isPropertySelected(propertyId) { return String(propertyId) === "property-1" }
        function setPropertySelected(propertyId, selected) {
            selectedPropertyCalls += 1
            lastPropertyId = propertyId
            lastPropertySelected = selected
        }
        function addProperty() { addPropertyCalls += 1 }
    }

    Component {
        id: panelComponent
        Import.TransactionDraftContractPropertyPanel {
            width: testCase.width
            theme: testCase.theme
            transactionViewModel: testCase.transactionViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_IMP_D_012_propertyPanelDelegatesSelectionAndQuickCreate() {
        const panel = createTemporaryObject(panelComponent, testCase)
        const check = findRequired(panel, "transactionDraftPropertyCheck_property-2")
        const input = findRequired(panel, "transactionDraftPropertyNameInput")

        check.checked = true
        check.toggled()
        input.text = "New Property"
        input.textEdited()
        findRequired(panel, "transactionDraftPropertyAddButton").clicked()

        compare(transactionViewModel.selectedPropertyCalls, 1)
        compare(transactionViewModel.lastPropertyId, "property-2")
        compare(transactionViewModel.lastPropertySelected, true)
        compare(transactionViewModel.propertyName, "New Property")
        compare(transactionViewModel.addPropertyCalls, 1)
    }
}
