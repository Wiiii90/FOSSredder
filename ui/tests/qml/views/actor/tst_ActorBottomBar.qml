/**
 * @file ui/tests/qml/views/actor/tst_ActorBottomBar.qml
 * @brief Provides QML tests for ActorBottomBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ActorBottomBarTests"
    when: windowShown
    width: 960
    height: 320

    property var theme: QtObject {
        property int viewActionButtonWidth: 120
        property int viewCompactActionButtonSize: 28
        property color textMuted: "#666666"
    }

    property var actorViewModel: QtObject {
        property bool isEdit: false
        property bool hasChanges: false
        readonly property bool canSubmit: name.trim().length > 0
        property string name: ""
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

    property var actorRows: [
        { id: "actor-1" }
    ]

    Component {
        id: bottomBarComponent
        ActorBottomBar {
            width: 960
            height: 64
            theme: testCase.theme
            actorViewModel: testCase.actorViewModel
            actorRows: testCase.actorRows
        }
    }

    function findRequired(root, objectName) {
        var found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function createBar() {
        return createTemporaryObject(bottomBarComponent, testCase)
    }

    function test_ACT_BB_001_navigationButtonsCallStateNavigation() {
        var bar = createBar()
        findRequired(bar, "actorPreviousButton").clicked()
        findRequired(bar, "actorNextButton").clicked()
        compare(actorViewModel.previousCalls, 1)
        compare(actorViewModel.nextCalls, 1)
    }

    function test_ACT_BB_002_createModeButtonsSwitchState() {
        var bar = createBar()
        actorViewModel.isEdit = false
        actorViewModel.name = "Alice"
        findRequired(bar, "actorClearButton").clicked()
        findRequired(bar, "actorCreateButton").clicked()
        compare(actorViewModel.clearCalls, 1)
        compare(actorViewModel.submitCalls, 1)
    }

    function test_ACT_BB_003_editModeButtonsSwitchState() {
        var bar = createBar()
        actorViewModel.isEdit = true
        actorViewModel.hasChanges = true
        actorViewModel.name = "Alice"
        findRequired(bar, "actorCreateModeButton").clicked()
        findRequired(bar, "actorDeleteButton").clicked()
        actorViewModel.submitCalls = 0
        findRequired(bar, "actorUpdateButton").clicked()
        compare(actorViewModel.enterCreateModeCalls, 1)
        compare(actorViewModel.deleteCurrentCalls, 1)
        compare(actorViewModel.submitCalls, 1)
    }
}
