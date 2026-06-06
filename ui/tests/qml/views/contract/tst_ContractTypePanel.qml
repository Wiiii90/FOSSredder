/**
 * @file ui/tests/qml/views/contract/tst_ContractTypePanel.qml
 * @brief Provides QML tests for ContractTypePanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Contract 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ContractTypePanelTests"
    when: windowShown
    width: 640
    height: 160

    property var contractViewModel: QtObject {
        property string type: ""
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: panelComponent
        ContractTypePanel {
            width: 640
            height: 160
            theme: testCase.theme
            contractViewModel: testCase.contractViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_CON_TP_001_typeFieldWritesState() {
        const panel = createTemporaryObject(panelComponent, testCase)
        const field = findRequired(panel, "contractTypeField")

        field.text = "lease"
        field.textEdited()

        compare(contractViewModel.type, "lease")
    }

    function test_CON_TP_002_typeFieldRendersState() {
        contractViewModel.type = "service"
        const panel = createTemporaryObject(panelComponent, testCase)

        compare(findRequired(panel, "contractTypeField").text, "service")
    }
}
