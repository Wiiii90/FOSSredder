/**
 * @file ui/tests/qml/views/property/tst_PropertyContractPanel.qml
 * @brief Provides QML tests for PropertyContractPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Property 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "PropertyContractPanelTests"
    when: windowShown
    width: 960
    height: 320

    property var contractRows: [
        { id: "contract-1", name: "Lease" },
        { id: "contract-2", name: "Rent" }
    ]

    property var propertyViewModel: QtObject {
        property var selectedContractIds: []
        function setContractSelected(contractId, selected) {
            var next = selectedContractIds ? selectedContractIds.slice(0) : []
            var id = String(contractId || "").trim()
            var index = next.indexOf(id)
            if (selected && index === -1)
                next.push(id)
            else if (!selected && index !== -1)
                next.splice(index, 1)
            selectedContractIds = next
        }
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: panelComponent
        PropertyContractPanel {
            width: 960
            height: 320
            theme: testCase.theme
            propertyViewModel: testCase.propertyViewModel
            contractRows: testCase.contractRows
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createPanel() {
        return createTemporaryObject(panelComponent, testCase)
    }

    function test_PROP_CP_001_selectionWritesState() {
        const panel = createPanel()
        const checkBox = findRequired(panel, "propertyContractCheckBox")

        checkBox.checked = true
        checkBox.toggled()

        compare(propertyViewModel.selectedContractIds.length, 1)
        compare(propertyViewModel.selectedContractIds[0], "contract-1")
    }

    function test_PROP_CP_002_selectionIsDerivedFromState() {
        propertyViewModel.selectedContractIds = ["contract-2"]
        const panel = createPanel()
        const checkBox = findRequired(panel, "propertyContractCheckBox")

        compare(checkBox.checked, false)
        propertyViewModel.selectedContractIds = ["contract-1"]
        compare(findRequired(panel, "propertyContractCheckBox").checked, true)
    }
}
