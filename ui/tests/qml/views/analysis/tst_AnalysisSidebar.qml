/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisSidebar.qml
 * @brief Provides QML tests for AnalysisSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnalysisSidebarTests"
    when: windowShown
    width: 260
    height: 180

    property string selectedId: ""

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacingMedium: 8
        property int shellSidebarPreferredWidth: 260
        property int viewSidebarRowHeight: 44
        property int viewSidebarRowRadius: 6
        property int viewSidebarRowSpacing: 2
        property int borderWidthThin: 1
        property color borderSoft: "#cccccc"
        property color selectionHighlight: "#aaccee"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    property var analysisState: QtObject {
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
            analysisState: testCase.analysisState
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
