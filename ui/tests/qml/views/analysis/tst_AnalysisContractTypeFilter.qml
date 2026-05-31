/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisContractTypeFilter.qml
 * @brief Provides QML tests for AnalysisContractTypeFilter behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnalysisContractTypeFilterTests"
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
    }

    property var analysisState: QtObject {
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

        function selectUnassignedContractTypes() {
            selectedContractTypes = ["unassigned"]
        }
    }

    Component {
        id: filterComponent

        Analysis.AnalysisContractTypeFilter {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            analysisState: testCase.analysisState
        }
    }

    function createFilter() {
        return createTemporaryObject(filterComponent, testCase)
    }

    function init() {
        analysisState.selectedContractTypes = ["lease", "unassigned"]
    }

    function test_ANL_CTF_001_contractTypeFilterActionsForwardSelectionToState() {
        const filter = createFilter()

        TestSupport.findRequired(Lookup, filter, "analysisContractTypeFilterUnassignedButton").clicked()
        compare(analysisState.selectedContractTypes.length, 1)
        compare(analysisState.selectedContractTypes[0], "unassigned")

        TestSupport.findRequired(Lookup, filter, "analysisContractTypeFilterAllButton").clicked()
        compare(analysisState.selectedContractTypes.length, 2)
        compare(analysisState.selectedContractTypes[0], "lease")

        const checkBox = TestSupport.findRequired(Lookup, filter, "analysisContractTypeFilterCheckBox")
        checkBox.checked = false
        checkBox.clicked()
        compare(analysisState.selectedContractTypes.length, 1)
        compare(analysisState.selectedContractTypes[0], "unassigned")
    }
}
