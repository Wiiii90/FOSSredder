/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisContractTypeFilter.qml
 * @brief Provides QML tests for AnalysisContractTypeFilter behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common"

TestCase {
    id: testCase
    name: "AnalysisContractTypeFilterTests"
    when: windowShown
    width: 360
    height: 220

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var analysisViewModel: QtObject {
        property var contractTypeRows: [{ value: "lease", label: "lease" }, { value: "unassigned", label: "Unassigned" }]
        property var selectedContractTypes: ["lease", "unassigned"]

        function setContractTypeSelected(type, selected) {
            let next = selectedContractTypes.slice()
            const existing = next.indexOf(type)
            if (selected && existing === -1)
                next.push(type)
            if (!selected && existing !== -1)
                next.splice(existing, 1)
            selectedContractTypes = next
        }

        function selectAllContractTypes() {
            selectedContractTypes = ["lease", "unassigned"]
        }

        function selectNoContractTypes() {
            selectedContractTypes = []
        }
    }

    Component {
        id: filterComponent

        Analysis.AnalysisContractTypeFilter {
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
        analysisViewModel.selectedContractTypes = ["lease", "unassigned"]
    }

    function test_ANL_CTF_001_contractTypeFilterActionsForwardSelectionToState() {
        const filter = createFilter()

        TestSupport.findRequired(Lookup, filter, "analysisContractTypeFilterNoneButton").clicked()
        compare(analysisViewModel.selectedContractTypes.length, 0)

        TestSupport.findRequired(Lookup, filter, "analysisContractTypeFilterAllButton").clicked()
        compare(analysisViewModel.selectedContractTypes.length, 2)
        compare(analysisViewModel.selectedContractTypes[0], "lease")

        const checkBox = TestSupport.findRequired(Lookup, filter, "analysisContractTypeFilterCheckBox")
        checkBox.checked = false
        checkBox.clicked()
        compare(analysisViewModel.selectedContractTypes.length, 1)
        compare(analysisViewModel.selectedContractTypes[0], "unassigned")
    }
}
