/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisPropertyFilter.qml
 * @brief Provides QML tests for AnalysisPropertyFilter behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common"

TestCase {
    id: testCase
    name: "AnalysisPropertyFilterTests"
    when: windowShown
    width: 360
    height: 220

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

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
