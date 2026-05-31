/**
 * @file ui/tests/qml/views/contract/tst_ContractAllocatablePanel.qml
 * @brief Provides QML tests for ContractAllocatablePanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ContractAllocatablePanelTests"
    when: windowShown
    width: 640
    height: 160

    property var contractState: QtObject {
        property string allocatableMode: "mixed"
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
        ContractAllocatablePanel {
            width: 640
            height: 160
            theme: testCase.theme
            contractState: testCase.contractState
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_CON_ALP_001_comboSelectionWritesState() {
        const panel = createTemporaryObject(panelComponent, testCase)
        const combo = findRequired(panel, "contractAllocatableModeCombo")

        combo.currentIndex = 1
        combo.activated(1)
        compare(contractState.allocatableMode, "allocatable")

        combo.currentIndex = 2
        combo.activated(2)
        compare(contractState.allocatableMode, "non-allocatable")
    }

    function test_CON_ALP_002_comboRendersState() {
        contractState.allocatableMode = "non-allocatable"
        const panel = createTemporaryObject(panelComponent, testCase)

        compare(findRequired(panel, "contractAllocatableModeCombo").currentIndex, 2)
    }
}
