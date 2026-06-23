/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisDateFilter.qml
 * @brief Provides QML tests for AnalysisDateFilter behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common"

TestCase {
    id: testCase
    name: "AnalysisDateFilterTests"
    when: windowShown
    width: 900
    height: 90

    property var analysisViewModel: QtObject {
        property int dateFieldIndex: 0
        property int dateModeIndex: 0
        property string yearValue: "2025"
        property string dateFromValue: ""
        property string dateToValue: ""
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: filterComponent

        Analysis.AnalysisDateFilter {
            width: testCase.width
            theme: testCase.theme
            analysisViewModel: testCase.analysisViewModel
        }
    }

    function createFilter() {
        return createTemporaryObject(filterComponent, testCase)
    }

    function init() {
        analysisViewModel.dateFieldIndex = 0
        analysisViewModel.dateModeIndex = 0
        analysisViewModel.yearValue = "2025"
        analysisViewModel.dateFromValue = ""
        analysisViewModel.dateToValue = ""
    }

    function test_ANL_DF_001_dateControlsWriteAnalysisState() {
        const filter = createFilter()

        TestSupport.findRequired(Lookup, filter, "analysisDateFieldComboBox").currentIndex = 1
        TestSupport.findRequired(Lookup, filter, "analysisDateModeComboBox").currentIndex = 1
        TestSupport.findRequired(Lookup, filter, "analysisDateFromField").text = "2026-01-01"
        TestSupport.findRequired(Lookup, filter, "analysisDateToField").text = "2026-12-31"

        compare(analysisViewModel.dateFieldIndex, 1)
        compare(analysisViewModel.dateModeIndex, 1)
        compare(analysisViewModel.dateFromValue, "2026-01-01")
        compare(analysisViewModel.dateToValue, "2026-12-31")
    }

    function test_ANL_DF_002_yearFieldWritesAnalysisState() {
        const filter = createFilter()

        TestSupport.findRequired(Lookup, filter, "analysisYearField").text = "2027"

        compare(analysisViewModel.yearValue, "2027")
    }
}
