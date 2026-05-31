/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisDateFilter.qml
 * @brief Provides QML tests for AnalysisDateFilter behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnalysisDateFilterTests"
    when: windowShown
    width: 900
    height: 90

    property var analysisState: QtObject {
        property int dateFieldIndex: 0
        property int dateModeIndex: 0
        property string yearValue: "2025"
        property string dateFromValue: ""
        property string dateToValue: ""
    }

    property var theme: QtObject {
        property int formFieldWidth: 180
        property int spacingSmall: 6
        property int panelPadding: 8
        property int radius: 3
        property int borderWidthThin: 1
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
    }

    Component {
        id: filterComponent

        Analysis.AnalysisDateFilter {
            width: testCase.width
            theme: testCase.theme
            analysisState: testCase.analysisState
        }
    }

    function createFilter() {
        return createTemporaryObject(filterComponent, testCase)
    }

    function init() {
        analysisState.dateFieldIndex = 0
        analysisState.dateModeIndex = 0
        analysisState.yearValue = "2025"
        analysisState.dateFromValue = ""
        analysisState.dateToValue = ""
    }

    function test_ANL_DF_001_dateControlsWriteAnalysisState() {
        const filter = createFilter()

        TestSupport.findRequired(Lookup, filter, "analysisDateFieldComboBox").currentIndex = 1
        TestSupport.findRequired(Lookup, filter, "analysisDateModeComboBox").currentIndex = 1
        TestSupport.findRequired(Lookup, filter, "analysisDateFromField").text = "2026-01-01"
        TestSupport.findRequired(Lookup, filter, "analysisDateToField").text = "2026-12-31"

        compare(analysisState.dateFieldIndex, 1)
        compare(analysisState.dateModeIndex, 1)
        compare(analysisState.dateFromValue, "2026-01-01")
        compare(analysisState.dateToValue, "2026-12-31")
    }

    function test_ANL_DF_002_yearFieldWritesAnalysisState() {
        const filter = createFilter()

        TestSupport.findRequired(Lookup, filter, "analysisYearField").text = "2027"

        compare(analysisState.yearValue, "2027")
    }
}
