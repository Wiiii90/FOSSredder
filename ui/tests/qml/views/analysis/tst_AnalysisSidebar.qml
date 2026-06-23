/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisSidebar.qml
 * @brief Provides QML tests for AnalysisSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common"

TestCase {
    id: testCase
    name: "AnalysisSidebarTests"
    when: windowShown
    width: 260
    height: 180

    property string selectedId: ""

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var analysisViewModel: QtObject {
        property string selectedAnalysisId: testCase.selectedId
        property var analysisRows: [
            { id: "analysis-1", name: "Income", type: "plot" },
            { id: "analysis-2", name: "Costs", type: "tab" }
        ]

        function selectAnalysis(id) {
            testCase.selectedId = id
        }
    }

    Component {
        id: sidebarComponent

        Analysis.AnalysisSidebar {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            analysisViewModel: testCase.analysisViewModel
        }
    }

    function createSidebar() {
        return createTemporaryObject(sidebarComponent, testCase)
    }

    function init() {
        selectedId = "analysis-1"
    }

    function test_ANL_SB_001_sidebarRowsSelectAnalysisState() {
        const sidebar = createSidebar()
        const clickArea = TestSupport.findRequired(Lookup, sidebar, "analysisSidebarRowMouseArea")

        mouseClick(clickArea, 4, 4)

        compare(selectedId, "analysis-1")
    }
}
