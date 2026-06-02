/**
 * @file ui/tests/qml/views/annual/tst_AnnualAnalysesPanel.qml
 * @brief Provides QML tests for AnnualAnalysesPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Annual 1.0 as Annual

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnnualAnalysesPanelTests"
    when: windowShown
    width: 960
    height: 420

    property var annualViewModel: QtObject {
        property var availableAnalysisRows: [
            { id: "analysis-1", display: "Analysis 1", typeLabel: "Table", exportFormatOptions: ["XLSX", "CSV"], exportFormatIndex: 0 }
        ]
        property var assignedAnalysisRows: [
            { id: "analysis-2", display: "Analysis 2", typeLabel: "Plot", exportFormatOptions: ["PNG", "JPG"], exportFormatIndex: 0 }
        ]
        property int addIndex: -1
        property string removedId: ""
        property string formatId: ""
        property string formatValue: ""
        function addAvailableAnalysisAtIndex(index) { addIndex = index }
        function removeAnalysis(id) { removedId = id }
        function setAnalysisExportFormat(id, exportFormat) { formatId = id; formatValue = exportFormat }
    }

    property var theme: QtObject {
        property int formLabelWidth: 120
        property int formFieldWidth: 200
        property int spacingSmall: 6
        property int viewSelectionPanelMinHeight: 160
        property int viewSelectionPanelPreferredHeight: 220
        property int controlHeight: 32
        property int viewCompactActionButtonSize: 28
        property int viewActionButtonWidth: 120
        property int radius: 3
        property int borderWidthThin: 1
        property color borderSoft: "#cccccc"
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: panelComponent
        Annual.AnnualAnalysesPanel {
            width: 960
            height: 420
            annualViewModel: testCase.annualViewModel
            theme: testCase.theme
        }
    }

    function createPanel() {
        return createTemporaryObject(panelComponent, testCase)
    }

    function init() {
        annualViewModel.addIndex = -1
        annualViewModel.removedId = ""
        annualViewModel.formatId = ""
        annualViewModel.formatValue = ""
    }

    function test_ANN_P_001_addAnnualAnalysisDelegatesToAnnualState() {
        const panel = createPanel()
        TestSupport.findRequired(Lookup, panel, "annualAddAnalysisButton").clicked()
        compare(annualViewModel.addIndex, 0)
    }

    function test_ANN_P_002_exportFormatUpdateDelegatesToAnnualState() {
        const panel = createPanel()
        const combo = TestSupport.findRequired(Lookup, panel, "annualAnalysisExportFormatComboBox")
        combo.currentIndex = 1
        combo.activated(1)
        compare(annualViewModel.formatId, "analysis-2")
        compare(annualViewModel.formatValue, "JPG")
    }

    function test_ANN_P_003_removeAnalysisDelegatesToAnnualState() {
        const panel = createPanel()
        TestSupport.findRequired(Lookup, panel, "annualRemoveAnalysisButton").clicked()
        compare(annualViewModel.removedId, "analysis-2")
    }
}
