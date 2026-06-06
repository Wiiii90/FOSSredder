/**
 * @file ui/tests/qml/views/export/tst_ExportView.qml
 * @brief Provides QML tests for ExportView behavior.
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
    name: "ExportViewTests"
    when: windowShown
    width: 960
    height: 640

    property var exportViewModel: QtObject {
        property string targetDirectory: "test:///export/default"
        property int packageFormatIndex: 0
        property string addMode: "annual"
        property var annualRows: [{ id: "annual-1", name: "Annual 1", display: "Annual 1" }]
        property var analysisRows: [{ id: "analysis-1", name: "Analysis 1", type: "tab" }]
        property var addRows: annualRows
        property string addTextRole: "display"
        property int selectedAddIndex: 0
        property bool canAddEntry: true
        property var exportEntries: []
        property bool canStart: true
        property bool showClear: true
        property bool showCancel: false
        property bool showPause: false
        property bool isPaused: false
        property string pauseText: "Pause"
        property real progress: 0.0
        property string statusText: "Ready"
        property bool hasError: false
        property int refreshCalls: 0
        property int startCalls: 0
        property int browseCalls: 0
        function refreshFromWorkspace() { refreshCalls += 1 }
        function browseDirectory() { browseCalls += 1 }
        function clearForm() {}
        function startExport() { startCalls += 1 }
        function cancelExport() {}
        function pauseExport() {}
        function resumeExport() {}
        function selectAddRow(index) { selectedAddIndex = index }
        function addSelectedEntry() {}
        function removeEntry(index) {}
        function updateAnnualEntryAtIndex(entryIndex, annualIndex) {}
        function updateAnnualCollapsed(entryIndex, collapsed) {}
        function updateStandaloneAnalysisAtIndex(entryIndex, analysisIndex) {}
        function updateStandaloneAnalysisExportType(entryIndex, exportType) {}
        function updateAnnualAnalysisExportType(entryIndex, analysisIndex, exportType) {}
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: exportViewComponent
        Export.ExportView {
            width: 960
            height: 640
            exportViewModel: testCase.exportViewModel
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(exportViewComponent, testCase)
    }

    function init() {
        exportViewModel.refreshCalls = 0
        exportViewModel.startCalls = 0
        exportViewModel.browseCalls = 0
    }

    function test_EXP_V_001_mountsExportCompositionWithExportState() {
        const view = createView()

        verify(TestSupport.findRequired(Lookup, view, "exportFormPanel") !== null)
        verify(TestSupport.findRequired(Lookup, view, "exportObjectsPanel") !== null)
        compare(exportViewModel.refreshCalls, 1)
    }

    function test_EXP_V_002_commandsFlowThroughInjectedExportState() {
        const view = createView()

        TestSupport.findRequired(Lookup, view, "exportBrowseDirectoryButton").clicked()
        TestSupport.findRequired(Lookup, view, "exportStartButton").clicked()

        compare(exportViewModel.browseCalls, 1)
        compare(exportViewModel.startCalls, 1)
    }
}
