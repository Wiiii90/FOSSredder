/**
 * @file ui/tests/qml/views/export/tst_ExportBottomBar.qml
 * @brief Provides QML tests for ExportBottomBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Export 1.0 as Export

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ExportBottomBarTests"
    when: windowShown
    width: 720
    height: 160

    property var exportViewModel: QtObject {
        property bool showClear: true
        property bool showCancel: false
        property bool showPause: false
        property bool isPaused: false
        property bool canStart: true
        property string pauseText: "Pause"
        property int clearCalls: 0
        property int startCalls: 0
        property int cancelCalls: 0
        property int pauseCalls: 0
        property int resumeCalls: 0
        function clearForm() { clearCalls += 1 }
        function startExport() { startCalls += 1 }
        function cancelExport() { cancelCalls += 1 }
        function pauseExport() { pauseCalls += 1 }
        function resumeExport() { resumeCalls += 1 }
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: exportBottomBarComponent
        Export.ExportBottomBar {
            width: 720
            height: 64
            exportViewModel: testCase.exportViewModel
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(exportBottomBarComponent, testCase)
    }

    function init() {
        exportViewModel.showClear = true
        exportViewModel.showCancel = false
        exportViewModel.showPause = false
        exportViewModel.isPaused = false
        exportViewModel.canStart = true
        exportViewModel.pauseText = "Pause"
        exportViewModel.clearCalls = 0
        exportViewModel.startCalls = 0
        exportViewModel.cancelCalls = 0
        exportViewModel.pauseCalls = 0
        exportViewModel.resumeCalls = 0
    }

    function test_EXP_BB_001_createModeButtonsDelegateToExportState() {
        const view = createView()

        TestSupport.findRequired(Lookup, view, "exportClearButton").clicked()
        TestSupport.findRequired(Lookup, view, "exportStartButton").clicked()

        compare(exportViewModel.clearCalls, 1)
        compare(exportViewModel.startCalls, 1)
    }

    function test_EXP_BB_002_progressModeButtonsDelegateToExportState() {
        exportViewModel.showClear = false
        exportViewModel.showCancel = true
        exportViewModel.showPause = true

        const view = createView()

        TestSupport.findRequired(Lookup, view, "exportCancelButton").clicked()
        TestSupport.findRequired(Lookup, view, "exportPauseResumeButton").clicked()

        compare(exportViewModel.cancelCalls, 1)
        compare(exportViewModel.pauseCalls, 1)
        compare(exportViewModel.resumeCalls, 0)
    }
}
