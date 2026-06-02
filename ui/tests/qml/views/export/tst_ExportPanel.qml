/**
 * @file ui/tests/qml/views/export/tst_ExportPanel.qml
 * @brief Provides QML tests for ExportPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Export 1.0 as Export

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ExportPanelTests"
    when: windowShown
    width: 960
    height: 640

    property var exportViewModel: QtObject {
        property string addMode: "annual"
        property var annualRows: [
            { id: "annual-1", name: "Annual 1", display: "Annual 1" },
            { id: "annual-2", name: "Annual 2", display: "Annual 2" }
        ]
        property var analysisRows: [
            { id: "analysis-1", name: "Analysis 1", type: "tab" },
            { id: "analysis-2", name: "Analysis 2", type: "plot" }
        ]
        property var addRows: addMode === "annual" ? annualRows : analysisRows
        property string addTextRole: addMode === "annual" ? "display" : "name"
        property int pendingIndex: 0
        property bool canAddEntry: true
        property var exportEntries: []
        property int addCalls: 0
        property int removeCalls: 0
        property int selectCalls: 0
        property int updateAnnualCalls: 0
        property int updateStandaloneCalls: 0
        property int updateExportTypeCalls: 0
        property int collapseCalls: 0
        function selectPendingRow(index) { pendingIndex = index; selectCalls += 1 }
        function addPendingEntry() {
            addCalls += 1
            if (addMode === "annual") {
                exportEntries = [{
                    isAnnual: true,
                    objectId: "annual-1",
                    objectName: "Annual 1",
                    annualIndex: 0,
                    collapsed: false,
                    analyses: [{
                        objectId: "analysis-1",
                        objectName: "Analysis 1",
                        analysisType: "tab",
                        exportType: "CSV",
                        exportTypeOptions: ["CSV", "XLSX"],
                        exportTypeIndex: 0
                    }]
                }]
                return
            }
            exportEntries = [{
                isAnnual: false,
                objectId: "analysis-2",
                objectName: "Analysis 2",
                analysisIndex: 1,
                analysisType: "plot",
                exportType: "PNG",
                exportTypeOptions: ["PNG", "JPG"],
                exportTypeIndex: 0
            }]
        }
        function removeEntry(index) { removeCalls += 1; exportEntries = [] }
        function updateAnnualEntryAtIndex(entryIndex, annualIndex) {
            updateAnnualCalls += 1
            exportEntries[entryIndex].annualIndex = annualIndex
            exportEntries = exportEntries
        }
        function updateAnnualCollapsed(entryIndex, collapsed) {
            collapseCalls += 1
            exportEntries[entryIndex].collapsed = collapsed
            exportEntries = exportEntries
        }
        function updateStandaloneAnalysisAtIndex(entryIndex, analysisIndex) {
            updateStandaloneCalls += 1
            exportEntries[entryIndex].analysisIndex = analysisIndex
            exportEntries = exportEntries
        }
        function updateStandaloneAnalysisExportType(entryIndex, exportType) {
            updateExportTypeCalls += 1
            exportEntries[entryIndex].exportType = exportType
            exportEntries = exportEntries
        }
        function updateAnnualAnalysisExportType(entryIndex, analysisIndex, exportType) {
            updateExportTypeCalls += 1
            exportEntries[entryIndex].analyses[analysisIndex].exportType = exportType
            exportEntries = exportEntries
        }
    }

    property var theme: QtObject {
        property int spacing: 8
        property int spacingSmall: 6
        property int formFieldWidth: 220
        property int controlHeight: 32
        property int viewCompactActionButtonSize: 28
        property int viewInlineIconSize: 28
        property int viewSectionIconSize: 42
        property int margins: 8
        property int radius: 3
        property int borderWidthThin: 1
        property int viewNavigationButtonWidth: 42
        property var exportView: ({
            panel: {
                addModeButtonWidth: 88,
                addButtonWidth: 72,
                panelMinHeight: 320,
                objectListMinHeight: 180,
                exportTypeColumnWidth: 110,
                kindColumnWidth: 88,
                analysisNameMinWidth: 160
            }
        })
        property color subtlePrimaryFill: "#eef3ff"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color borderSoft: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: exportPanelComponent
        Export.ExportPanel {
            width: 920
            height: 560
            exportViewModel: testCase.exportViewModel
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(exportPanelComponent, testCase)
    }

    function init() {
        exportViewModel.addMode = "annual"
        exportViewModel.pendingIndex = 0
        exportViewModel.exportEntries = []
        exportViewModel.addCalls = 0
        exportViewModel.removeCalls = 0
        exportViewModel.selectCalls = 0
        exportViewModel.updateAnnualCalls = 0
        exportViewModel.updateStandaloneCalls = 0
        exportViewModel.updateExportTypeCalls = 0
        exportViewModel.collapseCalls = 0
    }

    function test_EXP_P_001_addAnnualDelegatesToExportState() {
        const view = createView()
        TestSupport.findRequired(Lookup, view, "exportAddEntryButton").clicked()

        compare(exportViewModel.addCalls, 1)
        compare(exportViewModel.exportEntries.length, 1)
        compare(exportViewModel.exportEntries[0].isAnnual, true)
    }

    function test_EXP_P_002_addAnalysisModeDelegatesToExportState() {
        const view = createView()
        TestSupport.findRequired(Lookup, view, "exportAddAnalysisModeButton").clicked()
        TestSupport.findRequired(Lookup, view, "exportAddEntryButton").clicked()

        compare(exportViewModel.addMode, "analysis")
        compare(exportViewModel.exportEntries.length, 1)
        compare(exportViewModel.exportEntries[0].isAnnual, false)
    }

    function test_EXP_P_003_removeEntryDelegatesToExportState() {
        const view = createView()
        TestSupport.findRequired(Lookup, view, "exportAddEntryButton").clicked()
        wait(0)
        TestSupport.findRequired(Lookup, view, "exportRemoveAnnualButton").clicked()

        compare(exportViewModel.removeCalls, 1)
        compare(exportViewModel.exportEntries.length, 0)
    }

    function test_EXP_P_004_disclosureButtonDelegatesCollapseState() {
        const view = createView()
        TestSupport.findRequired(Lookup, view, "exportAddEntryButton").clicked()
        wait(0)
        TestSupport.findRequired(Lookup, view, "exportAnnualCollapseButton").clicked()

        compare(exportViewModel.collapseCalls, 1)
        compare(exportViewModel.exportEntries[0].collapsed, true)
    }

    function test_EXP_P_005_exportTypeDropdownDelegatesSelection() {
        const view = createView()
        TestSupport.findRequired(Lookup, view, "exportAddAnalysisModeButton").clicked()
        TestSupport.findRequired(Lookup, view, "exportAddEntryButton").clicked()
        wait(0)

        const exportTypeCombo = TestSupport.findRequired(
                    Lookup, view, "exportStandaloneAnalysisExportTypeComboBox")
        exportTypeCombo.currentIndex = 1
        exportTypeCombo.activated(1)

        compare(exportViewModel.updateExportTypeCalls, 1)
        compare(exportViewModel.exportEntries[0].exportType, "JPG")
    }
}
