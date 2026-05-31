/**
 * @file ui/tests/qml/views/export/tst_ExportSidebar.qml
 * @brief Provides QML tests for ExportSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Export 1.0 as Export

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ExportSidebarTests"
    when: windowShown
    width: 960
    height: 640

    property var exportState: QtObject {
        property var runs: []
        property int refreshCalls: 0
        property int openCalls: 0
        property int removeCalls: 0
        function refreshRuns() { refreshCalls += 1 }
        function openRunLocationAt(index) { openCalls += 1 }
        function removeRunAt(index) { removeCalls += 1 }
    }

    property var theme: QtObject {
        property int spacingMedium: 8
        property int spacingSmall: 6
        property int borderWidthThin: 1
        property int radius: 3
        property int viewSidebarRowHeight: 44
        property int viewSidebarRowRadius: 3
        property int viewCompactActionButtonSizeTiny: 20
        property color accent: "#3366ff"
        property color border: "#cccccc"
        property color borderSoft: "#cccccc"
        property color success: "#008800"
        property color warning: "#aa8800"
        property color danger: "#aa0000"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
    }

    Component {
        id: exportSidebarComponent
        Export.ExportSidebar {
            width: 960
            height: 640
            exportState: testCase.exportState
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(exportSidebarComponent, testCase)
    }

    function init() {
        exportState.runs = [
            {
                logId: "export-1",
                time: "2026-05-16 10:00:00",
                status: "Success",
                file: "/tmp/export.xlsx",
                message: "done",
                payload: "{\"items\":[]}",
                draftAttached: false,
                draftId: "",
                statementId: ""
            }
        ]
        exportState.refreshCalls = 0
        exportState.openCalls = 0
        exportState.removeCalls = 0
    }

    function test_EXP_S_001_runListIsBoundToExportStateRuns() {
        const view = createView()
        const runList = TestSupport.findRequired(Lookup, view, "runLogList")

        compare(exportState.refreshCalls, 1)
        compare(runList.count, 1)
    }
}
