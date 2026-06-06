/**
 * @file ui/tests/qml/views/export/tst_ExportSidebar.qml
 * @brief Provides QML tests for ExportSidebar behavior.
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
    name: "ExportSidebarTests"
    when: windowShown
    width: 960
    height: 640

    property var exportViewModel: QtObject {
        property var exportLogs: []
        property int openCalls: 0
        property int removeCalls: 0
        function openExportLogLocation(logId) { openCalls += 1 }
        function deleteExportLog(logId) { removeCalls += 1 }
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: exportSidebarComponent
        Export.ExportSidebar {
            width: 960
            height: 640
            exportViewModel: testCase.exportViewModel
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(exportSidebarComponent, testCase)
    }

    function init() {
        exportViewModel.exportLogs = [
            {
                logId: "export-1",
                time: "2026-05-16 10:00:00",
                status: "Success",
                file: "/tmp/export.xlsx",
                message: "done",
                displayTime: "2026-05-16 10:00:00",
                displayTitle: "export.xlsx",
                displayStatusDetail: "done",
                draftAttached: false,
                draftId: "",
                statementId: ""
            }
        ]
        exportViewModel.openCalls = 0
        exportViewModel.removeCalls = 0
    }

    function test_EXP_S_001_logListIsBoundToExportLogs() {
        const view = createView()
        const runList = TestSupport.findRequired(Lookup, view, "runLogList")

        compare(runList.count, 1)
    }
}
