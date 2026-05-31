/**
 * @file ui/tests/qml/views/actor/tst_ActorContractPanel.qml
 * @brief Provides QML tests for ActorContractPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ActorContractPanelTests"
    when: windowShown
    width: 960
    height: 320

    property var actorState: QtObject {
        property var selectedContractIds: []
        function isContractSelected(contractId) {
            return selectedContractIds.indexOf(String(contractId || "").trim()) !== -1
        }
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
        ActorContractPanel {
            width: 960
            height: 320
            theme: testCase.theme
            actorState: testCase.actorState
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

        compare(actorState.selectedContractIds.length, 1)
        compare(actorState.selectedContractIds[0], "contract-1")
    }

    function test_ACT_CP_002_selectionIsDerivedFromState() {
        actorState.selectedContractIds = ["contract-2"]
        var panel = createPanel()
        var checkBox = findRequired(panel, "actorContractCheckBox")

        compare(checkBox.checked, false)
        actorState.selectedContractIds = ["contract-1"]
        compare(findRequired(panel, "actorContractCheckBox").checked, true)
    }
}
