/**
 * @file ui/tests/qml/views/actor/tst_ActorContractPanel.qml
 * @brief Provides QML tests for ActorContractPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Actor 1.0

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ActorContractPanelTests"
    when: windowShown
    width: 960
    height: 320

    property var actorViewModel: QtObject {
        property var selectedContractIds: []
        function setContractSelected(contractId, selected) {
            const next = selected
                    ? selectedContractIds.concat([String(contractId || "").trim()]).filter(function(value, index, list) { return value.length > 0 && list.indexOf(value) === index })
                    : selectedContractIds.filter(function(value) { return String(value || "") !== String(contractId || "").trim() })
            selectedContractIds = next
        }
    }

    property var contractRows: [
        { id: "contract-1", name: "Lease" },
        { id: "contract-2", name: "Rent" }
    ]

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: panelComponent
        ActorContractPanel {
            width: 960
            height: 320
            theme: testCase.theme
            actorViewModel: testCase.actorViewModel
            contractRows: testCase.contractRows
        }
    }

    function findRequired(root, objectName) {
        var found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function createPanel() {
        return createTemporaryObject(panelComponent, testCase)
    }

    function test_ACT_CP_001_selectionWritesState() {
        var panel = createPanel()
        var checkBox = findRequired(panel, "actorContractCheckBox")

        checkBox.checked = true
        checkBox.toggled()

        compare(actorViewModel.selectedContractIds.length, 1)
        compare(actorViewModel.selectedContractIds[0], "contract-1")
    }

    function test_ACT_CP_002_selectionIsDerivedFromState() {
        actorViewModel.selectedContractIds = ["contract-2"]
        var panel = createPanel()
        var checkBox = findRequired(panel, "actorContractCheckBox")

        compare(checkBox.checked, false)
        actorViewModel.selectedContractIds = ["contract-1"]
        compare(findRequired(panel, "actorContractCheckBox").checked, true)
    }
}
