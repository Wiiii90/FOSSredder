/**
 * @file ui/tests/qml/views/contract/tst_ContractPropertiesPanel.qml
 * @brief Provides QML tests for ContractPropertiesPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ContractPropertiesPanelTests"
    when: windowShown
    width: 960
    height: 320

    property var propertyRows: [
        { id: "property-1", name: "Flat" },
        { id: "property-2", name: "House" }
    ]

    property var contractViewModel: QtObject {
        property var selectedPropertyIds: []
        function setPropertySelected(propertyId, selected) {
            var next = selectedPropertyIds ? selectedPropertyIds.slice(0) : []
            var id = String(propertyId || "").trim()
            var index = next.indexOf(id)
            if (selected && index === -1)
                next.push(id)
            else if (!selected && index !== -1)
                next.splice(index, 1)
            selectedPropertyIds = next
        }
    }

    property var theme: QtObject {
        property int viewSelectionPanelMinHeight: 80
        property int viewSelectionPanelPreferredHeight: 120
        property int spacingSmall: 6
        property int radius: 3
        property int borderWidthThin: 1
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
    }

    Component {
        id: panelComponent
        ContractPropertiesPanel {
            width: 960
            height: 320
            theme: testCase.theme
            contractViewModel: testCase.contractViewModel
            propertyRows: testCase.propertyRows
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createPanel() {
        return createTemporaryObject(panelComponent, testCase)
    }

    function test_CON_PP_001_selectionWritesState() {
        const panel = createPanel()
        const checkBox = findRequired(panel, "contractPropertyCheckBox")

        checkBox.checked = true
        checkBox.toggled()

        compare(contractViewModel.selectedPropertyIds.length, 1)
        compare(contractViewModel.selectedPropertyIds[0], "property-1")
    }

    function test_CON_PP_002_selectionIsDerivedFromState() {
        contractViewModel.selectedPropertyIds = ["property-2"]
        const panel = createPanel()
        const checkBox = findRequired(panel, "contractPropertyCheckBox")

        compare(checkBox.checked, false)
        contractViewModel.selectedPropertyIds = ["property-1"]
        compare(findRequired(panel, "contractPropertyCheckBox").checked, true)
    }
}
