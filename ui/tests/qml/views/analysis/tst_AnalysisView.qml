/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisView.qml
 * @brief Provides QML composition tests for AnalysisView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
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

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int viewFormSpacing: 8
        property int formLabelWidth: 120
        property int spacingSmall: 6
        property int spacing: 8
        property int panelPadding: 8
        property int panelContentSafeWidthOffset: 40
        property int controlHeight: 32
        property int formFieldWidth: 220
        property int viewSelectionPanelMinHeight: 160
        property int viewSelectionPanelPreferredHeight: 220
        property int viewActionButtonWidth: 120
        property int radius: 3
        property int borderWidthThin: 1
        property color borderSoft: "#cccccc"
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color borderLight: "#d7d7d7"
        property color borderStrong: "#888888"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color selectionHighlight: "#aaccee"
        property int chartPlotMinimumHeight: 180
        property int chartPlotPreferredHeight: 260
        property var analysis: QtObject {
            property var table: QtObject {
                property int propertyColumnWidth: 180
                property int amountColumnWidth: 120
                property int totalColumnWidth: 140
                property int rowHeight: 30
            }
            property var transactions: QtObject {
                property int applyColumnWidth: 44
                property int statementColumnWidth: 130
                property int transactionColumnWidth: 150
                property int dateColumnWidth: 110
                property int actorColumnWidth: 120
                property int contractColumnWidth: 120
                property int typeColumnWidth: 90
                property int propertiesColumnWidth: 220
                property int amountColumnWidth: 160
                property int columnSpacingCount: 9
                property int horizontalPaddingCount: 2
                property int headerHeight: 32
                property int rowHeight: 30
                property int calcPercentFieldWidth: 90
                property int metricsStatementWidth: 160
                property int metricsTransactionWidth: 170
                property int metricsAmountWidth: 180
            }
        }
    }

    property var analysisState: QtObject {
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
        property bool includeCalcAdjustments: true
        property bool filterEditMode: true
        property int filterWorkspaceIndex: 0
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
        property var previewTransactions: []
        property var previewTransactionRows: []
        property var previewMetrics: ({ statementCount: 0, transactionCount: 0, amountSum: 0.0 })
        property string previewStatementCountText: "Statements: 0"
        property string previewTransactionCountText: "Transactions: 0"
        property string previewAmountSumText: "Amount sum: 0.00"
        property var selectedAdjustmentTxIds: []
        property var adjustmentAmountsById: ({})
        property string calcName: ""
        property string calcPercentText: ""
        property string pendingAdjustmentsJson: "{}"
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
        function toggleFilterWorkspace() { filterWorkspaceIndex = filterWorkspaceIndex === 0 ? 1 : 0 }
        function clearFilters() {}
        function submitCreate() { testCase.submitCreateCalls += 1 }
        function submitUpdate() {}
        function deleteCurrent() {}
        function applySelectedCalc() {}
        function isPropertySelected(id) { return selectedPropertyIds.indexOf(id) !== -1 }
        function setPropertySelected(id, selected) {}
        function selectAllProperties() {}
        function selectNoProperties() {}
        function selectUnassignedProperties() {}
        function isContractTypeSelected(type) { return selectedContractTypes.indexOf(type) !== -1 }
        function setContractTypeSelected(type, selected) {}
        function selectAllContractTypes() {}
        function selectNoContractTypes() {}
        function selectUnassignedContractTypes() {}
        function isAdjustmentTransactionSelected(id) { return selectedAdjustmentTxIds.indexOf(id) !== -1 }
        function setAdjustmentTransactionSelected(id, selected) {}
        function setAllocatableModeIndex(index) {}
    }

    Component {
        id: viewComponent

        Analysis.AnalysisView {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            analysisState: testCase.analysisState
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
        analysisState.isEdit = false
        analysisState.name = ""
        analysisState.filterWorkspaceIndex = 0
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
