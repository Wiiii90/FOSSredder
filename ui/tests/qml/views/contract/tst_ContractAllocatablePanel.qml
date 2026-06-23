/**
 * @file ui/tests/qml/views/contract/tst_ContractAllocatablePanel.qml
 * @brief Provides QML tests for ContractAllocatablePanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Contract 1.0

import "../../common"

TestCase {
    id: testCase
    name: "ContractAllocatablePanelTests"
    when: windowShown
    width: 640
    height: 160

    property var contractViewModel: QtObject {
        property string allocatableMode: "mixed"
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: panelComponent
        ContractAllocatablePanel {
            width: 640
            height: 160
            theme: testCase.theme
            contractViewModel: testCase.contractViewModel
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
        compare(contractViewModel.allocatableMode, "allocatable")

        combo.currentIndex = 2
        combo.activated(2)
        compare(contractViewModel.allocatableMode, "non-allocatable")
    }

    function test_CON_ALP_002_comboRendersState() {
        contractViewModel.allocatableMode = "non-allocatable"
        const panel = createTemporaryObject(panelComponent, testCase)

        compare(findRequired(panel, "contractAllocatableModeCombo").currentIndex, 2)
    }
}
