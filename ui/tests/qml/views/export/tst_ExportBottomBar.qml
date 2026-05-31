/**
 * @file ui/tests/qml/views/export/tst_ExportBottomBar.qml
 * @brief Provides QML tests for ExportBottomBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Export 1.0 as Export

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ExportBottomBarTests"
    when: windowShown
    width: 720
    height: 160

    property var exportState: QtObject {
        property bool showClear: true
        property bool showCancel: false
        property bool showPause: false
        property bool canStart: true
        property string pauseText: "Pause"
        property int clearCalls: 0
        property int startCalls: 0
        property int cancelCalls: 0
        property int pauseCalls: 0
        function clearForm() { clearCalls += 1 }
        function startExport() { startCalls += 1 }
        function cancelExport() { cancelCalls += 1 }
        function togglePause() { pauseCalls += 1 }
    }

    property var theme: QtObject {
        property int spacing: 8
        property int spacingSmall: 6
        property int viewActionButtonWidth: 120
    }

    Component {
        id: exportBottomBarComponent
        Export.ExportBottomBar {
            width: 720
            height: 64
            exportState: testCase.exportState
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(exportBottomBarComponent, testCase)
    }

    function init() {
        exportState.showClear = true
        exportState.showCancel = false
        exportState.showPause = false
        exportState.canStart = true
        exportState.pauseText = "Pause"
        exportState.clearCalls = 0
        exportState.startCalls = 0
        exportState.cancelCalls = 0
        exportState.pauseCalls = 0
    }

    function test_EXP_BB_001_createModeButtonsDelegateToExportState() {
        const view = createView()

        TestSupport.findRequired(Lookup, view, "exportClearButton").clicked()
        TestSupport.findRequired(Lookup, view, "exportStartButton").clicked()

        compare(exportState.clearCalls, 1)
        compare(exportState.startCalls, 1)
    }

    function test_EXP_BB_002_progressModeButtonsDelegateToExportState() {
        exportState.showClear = false
        exportState.showCancel = true
        exportState.showPause = true

        const view = createView()

        TestSupport.findRequired(Lookup, view, "exportCancelButton").clicked()
        TestSupport.findRequired(Lookup, view, "exportTogglePauseButton").clicked()

        compare(exportState.cancelCalls, 1)
        compare(exportState.pauseCalls, 1)
    }
}
