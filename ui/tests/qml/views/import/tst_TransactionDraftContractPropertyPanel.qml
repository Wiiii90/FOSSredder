/**
 * @file ui/tests/qml/views/import/tst_TransactionDraftContractPropertyPanel.qml
 * @brief Provides QML tests for TransactionDraftContractPropertyPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "TransactionDraftContractPropertyPanelTests"
    when: windowShown
    width: 900
    height: 320

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var importViewModel: QtObject {
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
            importViewModel: testCase.importViewModel
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

        compare(importViewModel.selectedPropertyCalls, 1)
        compare(importViewModel.lastPropertyId, "property-2")
        compare(importViewModel.lastPropertySelected, true)
        compare(importViewModel.propertyName, "New Property")
        compare(importViewModel.addPropertyCalls, 1)
    }
}
