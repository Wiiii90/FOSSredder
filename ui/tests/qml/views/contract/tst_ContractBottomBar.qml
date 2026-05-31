/**
 * @file ui/tests/qml/views/contract/tst_ContractBottomBar.qml
 * @brief Provides QML tests for ContractBottomBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ContractBottomBarTests"
    when: windowShown
    width: 960
    height: 320

    property var theme: QtObject {
        property int viewActionButtonWidth: 120
        property int viewCompactActionButtonSize: 28
        property color textMuted: "#666666"
    }

    property var contractState: QtObject {
        property bool isEdit: false
        property bool hasChanges: false
        property string name: ""
        property string type: ""
        property var selectedActorIds: ["actor-1"]
        property var selectedPropertyIds: []
        readonly property bool canSubmit: name.trim().length > 0
                                          && type.trim().length > 0
                                          && (selectedActorIds.length > 0 || selectedPropertyIds.length > 0)
        property int previousCalls: 0
        property int nextCalls: 0
        property int clearCalls: 0
        property int submitCalls: 0
        property int enterCreateModeCalls: 0
        property int deleteCurrentCalls: 0
        function previous() { previousCalls += 1 }
        function next() { nextCalls += 1 }
        function clear() { clearCalls += 1 }
        function submit() { submitCalls += 1 }
        function enterCreateMode() { enterCreateModeCalls += 1 }
        function deleteCurrent() { deleteCurrentCalls += 1 }
    }

    property var contractRows: [
        { id: "contract-1" }
    ]

    Component {
        id: bottomBarComponent
        ContractBottomBar {
            width: 960
            height: 64
            theme: testCase.theme
            contractState: testCase.contractState
            contractRows: testCase.contractRows
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createBar() {
        return createTemporaryObject(bottomBarComponent, testCase)
    }

    function test_CON_BB_001_navigationButtonsCallStateNavigation() {
        const bar = createBar()
        findRequired(bar, "contractPreviousButton").clicked()
        findRequired(bar, "contractNextButton").clicked()
        compare(contractState.previousCalls, 1)
        compare(contractState.nextCalls, 1)
    }

    function test_CON_BB_002_createModeButtonsSwitchState() {
        const bar = createBar()
        contractState.isEdit = false
        contractState.name = "Lease"
        contractState.type = "rent"
        findRequired(bar, "contractClearButton").clicked()
        findRequired(bar, "contractCreateButton").clicked()
        compare(contractState.clearCalls, 1)
        compare(contractState.submitCalls, 1)
    }

    function test_CON_BB_003_editModeButtonsSwitchState() {
        const bar = createBar()
        contractState.isEdit = true
        contractState.hasChanges = true
        contractState.name = "Lease"
        contractState.type = "rent"
        findRequired(bar, "contractCreateModeButton").clicked()
        findRequired(bar, "contractDeleteButton").clicked()
        contractState.submitCalls = 0
        findRequired(bar, "contractUpdateButton").clicked()
        compare(contractState.enterCreateModeCalls, 1)
        compare(contractState.deleteCurrentCalls, 1)
        compare(contractState.submitCalls, 1)
    }
}
