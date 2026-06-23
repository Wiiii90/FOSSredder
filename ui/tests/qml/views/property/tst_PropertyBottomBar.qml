/**
 * @file ui/tests/qml/views/property/tst_PropertyBottomBar.qml
 * @brief Provides QML tests for PropertyBottomBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Property 1.0

import "../../common"

TestCase {
    id: testCase
    name: "PropertyBottomBarTests"
    when: windowShown
    width: 960
    height: 320

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var propertyViewModel: QtObject {
        property bool isEdit: false
        property bool hasChanges: false
        property string name: ""
        readonly property bool canSubmit: name.trim().length > 0
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

    property var propertyRows: [
        { id: "property-1" }
    ]

    Component {
        id: bottomBarComponent
        PropertyBottomBar {
            width: 960
            height: 64
            theme: testCase.theme
            propertyViewModel: testCase.propertyViewModel
            propertyRows: testCase.propertyRows
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createBar() {
        return createTemporaryObject(bottomBarComponent, testCase)
    }

    function test_PROP_BB_001_navigationButtonsCallStateNavigation() {
        const bar = createBar()
        findRequired(bar, "propertyPreviousButton").clicked()
        findRequired(bar, "propertyNextButton").clicked()
        compare(propertyViewModel.previousCalls, 1)
        compare(propertyViewModel.nextCalls, 1)
    }

    function test_PROP_BB_002_createModeButtonsSwitchState() {
        const bar = createBar()
        propertyViewModel.isEdit = false
        propertyViewModel.name = "Flat"
        findRequired(bar, "propertyClearButton").clicked()
        findRequired(bar, "propertyCreateButton").clicked()
        compare(propertyViewModel.clearCalls, 1)
        compare(propertyViewModel.submitCalls, 1)
    }

    function test_PROP_BB_003_editModeButtonsSwitchState() {
        const bar = createBar()
        propertyViewModel.isEdit = true
        propertyViewModel.hasChanges = true
        propertyViewModel.name = "Flat"
        findRequired(bar, "propertyDeleteButton").clicked()
        propertyViewModel.submitCalls = 0
        findRequired(bar, "propertyUpdateButton").clicked()
        compare(propertyViewModel.deleteCurrentCalls, 1)
        compare(propertyViewModel.submitCalls, 1)
    }
}
