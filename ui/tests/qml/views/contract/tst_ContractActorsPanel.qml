/**
 * @file ui/tests/qml/views/contract/tst_ContractActorsPanel.qml
 * @brief Provides QML tests for ContractActorsPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ContractActorsPanelTests"
    when: windowShown
    width: 960
    height: 320

    property var actorRows: [
        { id: "actor-1", name: "Alice" },
        { id: "actor-2", name: "Bob" }
    ]

    property var contractState: QtObject {
        property var selectedActorIds: []
        readonly property var actorDisplayRows: [
            { id: "", display: "No actor" },
            { id: "actor-1", display: "Alice" },
            { id: "actor-2", display: "Bob" }
        ]
        readonly property int selectedActorIndex: {
            const selectedId = selectedActorIds.length > 0 ? selectedActorIds[0] : ""
            for (let i = 0; i < actorDisplayRows.length; ++i) {
                if (String(actorDisplayRows[i].id || "") === String(selectedId || ""))
                    return i
            }
            return 0
        }
        function selectPrimaryActor(actorId) {
            var id = String(actorId || "").trim()
            selectedActorIds = id.length > 0 ? [id] : []
        }
    }

    property var theme: QtObject {
        property int spacingSmall: 6
        property int radius: 3
        property int borderWidthThin: 1
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
    }

    Component {
        id: panelComponent
        ContractActorsPanel {
            width: 960
            height: 120
            theme: testCase.theme
            contractState: testCase.contractState
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createPanel() {
        return createTemporaryObject(panelComponent, testCase)
    }

    function init() {
        contractState.selectedActorIds = []
    }

    function test_CON_AP_001_dropdownSelectionWritesSelectedActorId() {
        var panel = createPanel()
        var comboBox = findRequired(panel, "contractActorComboBox")

        comboBox.currentIndex = 1
        comboBox.activated(1)

        compare(contractState.selectedActorIds.length, 1)
        compare(contractState.selectedActorIds[0], "actor-1")
    }

    function test_CON_AP_002_existingSelectionIsRendered() {
        contractState.selectedActorIds = ["actor-2"]
        var panel = createPanel()
        var comboBox = findRequired(panel, "contractActorComboBox")

        compare(comboBox.currentIndex > 0, true)
    }
}
