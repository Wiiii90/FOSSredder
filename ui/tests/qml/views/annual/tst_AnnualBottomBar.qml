/**
 * @file ui/tests/qml/views/annual/tst_AnnualBottomBar.qml
 * @brief Provides QML tests for AnnualBottomBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Annual 1.0 as Annual

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnnualBottomBarTests"
    when: windowShown
    width: 960
    height: 120

    property var annualViewModel: QtObject {
        property bool isEdit: false
        property bool canSubmit: true
        property bool hasRows: true
        property bool hasChanges: true
        property int navigateCalls: 0
        property int lastDelta: 0
        property int toggleCalls: 0
        property int resetCalls: 0
        property int createCalls: 0
        property int updateCalls: 0
        property int deleteCalls: 0
        function navigate(delta) { navigateCalls += 1; lastDelta = delta }
        function toggleWorkspace() { toggleCalls += 1 }
        function resetCreateState() { resetCalls += 1 }
        function submitCreate() { createCalls += 1 }
        function submitUpdate() { updateCalls += 1 }
        function deleteCurrent() { deleteCalls += 1 }
    }

    property var theme: QtObject {
        property int viewNavigationButtonWidth: 42
        property int viewActionButtonWidth: 120
        property int spacing: 8
        property int spacingSmall: 6
        property int controlHeight: 32
        property int radius: 3
        property int borderWidthThin: 1
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color danger: "#b0302f"
        property color success: "#0a7f2e"
    }

    Component {
        id: bottomBarComponent
        Annual.AnnualBottomBar {
            width: 960
            height: 80
            annualViewModel: testCase.annualViewModel
            theme: testCase.theme
        }
    }

    function createBar() {
        return createTemporaryObject(bottomBarComponent, testCase)
    }

    function init() {
        annualViewModel.isEdit = false
        annualViewModel.navigateCalls = 0
        annualViewModel.toggleCalls = 0
        annualViewModel.resetCalls = 0
        annualViewModel.createCalls = 0
        annualViewModel.updateCalls = 0
        annualViewModel.deleteCalls = 0
    }

    function test_ANN_BB_001_createModeActionsDelegateToAnnualState() {
        const bar = createBar()
        TestSupport.findRequired(Lookup, bar, "annualPreviousButton").clicked()
        TestSupport.findRequired(Lookup, bar, "annualToggleWorkspaceButton").clicked()
        TestSupport.findRequired(Lookup, bar, "annualClearButton").clicked()
        TestSupport.findRequired(Lookup, bar, "annualCreateButton").clicked()
        TestSupport.findRequired(Lookup, bar, "annualNextButton").clicked()

        compare(annualViewModel.navigateCalls, 2)
        compare(annualViewModel.toggleCalls, 1)
        compare(annualViewModel.resetCalls, 1)
        compare(annualViewModel.createCalls, 1)
    }

    function test_ANN_BB_002_editModeActionsDelegateToAnnualState() {
        annualViewModel.isEdit = true
        const bar = createBar()
        TestSupport.findRequired(Lookup, bar, "annualDeleteButton").clicked()
        TestSupport.findRequired(Lookup, bar, "annualUpdateButton").clicked()

        compare(annualViewModel.deleteCalls, 1)
        compare(annualViewModel.updateCalls, 1)
    }
}
