/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisAllocatableFilter.qml
 * @brief Provides QML tests for AnalysisAllocatableFilter behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnalysisAllocatableFilterTests"
    when: windowShown
    width: 420
    height: 90

    property int lastAllocatableIndex: -1

    property var theme: QtObject {
        property int formLabelWidth: 120
        property int formFieldWidth: 220
        property int spacingSmall: 6
        property int panelPadding: 8
        property int radius: 3
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
    }

    property var analysisState: QtObject {
        function setAllocatableModeIndex(index) {
            testCase.lastAllocatableIndex = index
        }
    }

    Component {
        id: filterComponent

        Analysis.AnalysisAllocatableFilter {
            width: testCase.width
            theme: testCase.theme
            analysisState: testCase.analysisState
            mode: "all"
        }
    }

    function createFilter() {
        return createTemporaryObject(filterComponent, testCase)
    }

    function init() {
        lastAllocatableIndex = -1
    }

    function test_ANL_AF_001_allocatableModeSelectionForwardsIndexToState() {
        const filter = createFilter()
        const combo = TestSupport.findRequired(Lookup, filter, "analysisAllocatableModeComboBox")

        combo.currentIndex = 2

        compare(lastAllocatableIndex, 2)
    }
}
