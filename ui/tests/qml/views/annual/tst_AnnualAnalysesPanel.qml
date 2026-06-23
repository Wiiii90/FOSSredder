/**
 * @file ui/tests/qml/views/annual/tst_AnnualAnalysesPanel.qml
 * @brief Provides QML tests for AnnualAnalysesPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Annual 1.0 as Annual

import "../../common"

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

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

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
