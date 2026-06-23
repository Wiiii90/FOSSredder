/**
 * @file ui/tests/qml/views/export/tst_ExportProgressBar.qml
 * @brief Provides QML tests for ExportProgressBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Export 1.0 as Export

import "../../common"

TestCase {
    id: testCase
    name: "ExportProgressBarTests"
    when: windowShown
    width: 960
    height: 640

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var exportViewModel: QtObject {
        property real progress: 0.0
        property string statusText: "Ready"
        property bool hasError: false
    }

    Component {
        id: exportProgressBarComponent
        Export.ExportProgressBar {
            width: 700
            theme: testCase.theme
            exportViewModel: testCase.exportViewModel
        }
    }

    function createView() {
        return createTemporaryObject(exportProgressBarComponent, testCase)
    }

    function init() {
        exportViewModel.progress = 0.0
        exportViewModel.statusText = "Ready"
        exportViewModel.hasError = false
    }

    function test_EXP_PB_001_progressAndStatusFollowExportState() {
        exportViewModel.progress = 0.65
        exportViewModel.statusText = "Exporting"

        const view = createView()
        const bar = TestSupport.findRequired(Lookup, view, "exportProgressBar")
        const statusLabel = TestSupport.findRequired(Lookup, view, "exportProgressStatusLabel")

        compare(bar.value, 0.65)
        compare(statusLabel.text, "Exporting")
    }

    function test_EXP_PB_002_errorStateUsesDangerColor() {
        exportViewModel.statusText = "Disk full"
        exportViewModel.hasError = true

        const view = createView()
        const statusLabel = TestSupport.findRequired(Lookup, view, "exportProgressStatusLabel")

        compare(statusLabel.text, "Disk full")
        compare(statusLabel.color, theme.danger)
    }
}
