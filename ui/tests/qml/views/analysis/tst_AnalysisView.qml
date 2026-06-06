/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisView.qml
 * @brief Provides QML composition tests for AnalysisView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnalysisViewTests"
    when: windowShown
    width: 960
    height: 640

    property int refreshCalls: 0
    property int submitCreateCalls: 0
    property int navigateCalls: 0

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var analysisViewModel: QtObject {
        property bool isEdit: false
        property string selectedAnalysisId: ""
        property var analysisRows: [{ id: "analysis-1", name: "A1" }]
        property string name: ""
        property int mainTypeIndex: 0
        property int plotSubtypeIndex: 0
        property var plotTypeOptions: [{ value: "pie", label: "Pie chart" }]
        property var exportFormatOptions: [{ value: "png", label: "PNG" }]
        property string exportFormat: "png"
        property int exportFormatIndex: 0
        property bool includeAdjustments: true
        property bool filterEditMode: true
        property int filterContentIndex: 0
        property int dateFieldIndex: 0
        property int dateModeIndex: 0
        property string yearValue: "2025"
        property string dateFromValue: ""
        property string dateToValue: ""
        property var propertyFilterRows: []
        property var contractTypeRows: []
        property var selectedPropertyIds: []
        property var selectedContractTypes: []
        property string allocatableMode: "all"
        property var previewTransactionRows: []
        property string previewStatementCountText: "Statements: 0"
        property string previewTransactionCountText: "Transactions: 0"
        property string previewAmountSumText: "Amount sum: 0.00"
        property var selectedAdjustmentTxIds: []
        property string adjustmentName: ""
        property string adjustmentPercentText: ""
        property string currentResultType: "plot"
        property bool currentResultIsTable: false
        property string renderedPreviewSource: ""
        property var tableContractTypes: []
        property var tablePropertyRows: []
        property real tableGrandTotal: 0.0
        property bool canSubmit: true
        property bool hasRows: true

        function refreshFromSelection() { testCase.refreshCalls += 1 }
        function navigate(delta) { testCase.navigateCalls += 1 }
        function toggleFilterContent() { filterContentIndex = filterContentIndex === 0 ? 1 : 0 }
        function clearFilters() {}
        function submitCreate() { testCase.submitCreateCalls += 1 }
        function submitUpdate() {}
        function deleteCurrent() {}
        function applySelectedAdjustment() {}
        function isPropertySelected(id) { return selectedPropertyIds.indexOf(id) !== -1 }
        function setPropertySelected(id, selected) {}
        function selectAllProperties() {}
        function selectNoProperties() {}
        function isContractTypeSelected(type) { return selectedContractTypes.indexOf(type) !== -1 }
        function setContractTypeSelected(type, selected) {}
        function selectAllContractTypes() {}
        function selectNoContractTypes() {}
        function setAdjustmentTransactionSelected(id, selected) {}
        function setAllocatableModeIndex(index) {}
    }

    Component {
        id: viewComponent

        Analysis.AnalysisView {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            analysisViewModel: testCase.analysisViewModel
        }
    }

    function createView() {
        const view = createTemporaryObject(viewComponent, testCase)
        view.visible = true
        return view
    }

    function init() {
        refreshCalls = 0
        submitCreateCalls = 0
        navigateCalls = 0
        analysisViewModel.isEdit = false
        analysisViewModel.name = ""
        analysisViewModel.filterContentIndex = 0
    }

    function test_ANL_V_001_containerRefreshCallsState() {
        const view = createView()

        verify(refreshCalls > 0)
        verify(TestSupport.findRequired(Lookup, view, "analysisNameField") !== null)
    }

    function test_ANL_V_002_bottomBarActionsReachState() {
        const view = createView()

        TestSupport.findRequired(Lookup, view, "analysisCreateButton").clicked()
        TestSupport.findRequired(Lookup, view, "analysisNextButton").clicked()

        compare(submitCreateCalls, 1)
        compare(navigateCalls, 1)
    }
}
