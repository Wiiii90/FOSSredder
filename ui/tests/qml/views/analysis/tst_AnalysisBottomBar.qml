/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisBottomBar.qml
 * @brief Provides QML tests for AnalysisBottomBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnalysisBottomBarTests"
    when: windowShown
    width: 640
    height: 120

    property int createCalls: 0
    property int updateCalls: 0
    property int deleteCalls: 0
    property int resetCalls: 0
    property int navigateCalls: 0

    property var theme: QtObject {
        property int spacing: 8
        property int spacingSmall: 6
        property int radius: 3
        property int borderWidthThin: 1
        property int viewActionButtonWidth: 96
        property color surface: "#ffffff"
        property color border: "#cccccc"
    }

    property var analysisState: QtObject {
        property bool isEdit: false
        property bool filterEditMode: true
        property bool canSubmit: true
        property bool hasRows: true
        function navigate(delta) { testCase.navigateCalls += 1 }
        function toggleFilterWorkspace() {}
        function clearFilters() { testCase.resetCalls += 1 }
        function submitCreate() { testCase.createCalls += 1 }
        function submitUpdate() { testCase.updateCalls += 1 }
        function deleteCurrent() { testCase.deleteCalls += 1 }
    }

    Component {
        id: barComponent

        Analysis.AnalysisBottomBar {
            width: testCase.width
            theme: testCase.theme
            analysisState: testCase.analysisState
        }
    }

    function createBar() {
        return createTemporaryObject(barComponent, testCase)
    }

    function init() {
        createCalls = 0
        updateCalls = 0
        deleteCalls = 0
        resetCalls = 0
        navigateCalls = 0
        analysisState.isEdit = false
    }

    function test_ANL_BB_001_createModeActionsForwardToState() {
        const bar = createBar()

        TestSupport.findRequired(Lookup, bar, "analysisResetButton").clicked()
        TestSupport.findRequired(Lookup, bar, "analysisCreateButton").clicked()
        TestSupport.findRequired(Lookup, bar, "analysisNextButton").clicked()

        compare(resetCalls, 1)
        compare(createCalls, 1)
        compare(navigateCalls, 1)
    }

    function test_ANL_BB_002_editModeActionsForwardToState() {
        analysisState.isEdit = true
        const bar = createBar()

        TestSupport.findRequired(Lookup, bar, "analysisUpdateButton").clicked()
        TestSupport.findRequired(Lookup, bar, "analysisDeleteButton").clicked()

        compare(updateCalls, 1)
        compare(deleteCalls, 1)
    }
}
