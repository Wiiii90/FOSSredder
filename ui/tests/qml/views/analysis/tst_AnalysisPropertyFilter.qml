/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisPropertyFilter.qml
 * @brief Provides QML tests for AnalysisPropertyFilter behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnalysisPropertyFilterTests"
    when: windowShown
    width: 360
    height: 220

    property var theme: QtObject {
        property int viewSelectionPanelMinHeight: 120
        property int viewSelectionPanelPreferredHeight: 180
        property int viewCompactActionButtonSize: 28
        property int controlHeight: 32
        property int spacingSmall: 6
        property int panelPadding: 8
        property int radius: 3
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
    }

    property var analysisViewModel: QtObject {
        property var propertyFilterRows: [{ id: "property-1", name: "Lot" }, { id: "unassigned", name: "Unassigned" }]
        property var selectedPropertyIds: ["property-1", "unassigned"]

        function setPropertySelected(id, selected) {
            let next = selectedPropertyIds.slice()
            const existing = next.indexOf(id)
            if (selected && existing === -1)
                next.push(id)
            if (!selected && existing !== -1)
                next.splice(existing, 1)
            selectedPropertyIds = next
        }

        function selectAllProperties() {
            selectedPropertyIds = ["property-1", "unassigned"]
        }

        function selectNoProperties() {
            selectedPropertyIds = []
        }
    }

    Component {
        id: filterComponent

        Analysis.AnalysisPropertyFilter {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            analysisViewModel: testCase.analysisViewModel
        }
    }

    function createFilter() {
        return createTemporaryObject(filterComponent, testCase)
    }

    function init() {
        analysisViewModel.selectedPropertyIds = ["property-1", "unassigned"]
    }

    function test_ANL_PF_001_propertyFilterActionsForwardSelectionToState() {
        const filter = createFilter()

        TestSupport.findRequired(Lookup, filter, "analysisPropertyFilterNoneButton").clicked()
        compare(analysisViewModel.selectedPropertyIds.length, 0)

        TestSupport.findRequired(Lookup, filter, "analysisPropertyFilterAllButton").clicked()
        compare(analysisViewModel.selectedPropertyIds.length, 2)
        compare(analysisViewModel.selectedPropertyIds[0], "property-1")

        const checkBox = TestSupport.findRequired(Lookup, filter, "analysisPropertyFilterCheckBox")
        checkBox.checked = false
        checkBox.clicked()
        compare(analysisViewModel.selectedPropertyIds.length, 1)
        compare(analysisViewModel.selectedPropertyIds[0], "unassigned")
    }
}
