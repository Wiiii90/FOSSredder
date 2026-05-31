/**
 * @file ui/tests/qml/views/contract/tst_ContractTypePanel.qml
 * @brief Provides QML tests for ContractTypePanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ContractTypePanelTests"
    when: windowShown
    width: 640
    height: 160

    property var contractState: QtObject {
        property string type: ""
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
        ContractTypePanel {
            width: 640
            height: 160
            theme: testCase.theme
            contractState: testCase.contractState
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

        compare(contractState.type, "lease")
    }

    function test_CON_TP_002_typeFieldRendersState() {
        contractState.type = "service"
        const panel = createTemporaryObject(panelComponent, testCase)

        compare(findRequired(panel, "contractTypeField").text, "service")
    }
}
