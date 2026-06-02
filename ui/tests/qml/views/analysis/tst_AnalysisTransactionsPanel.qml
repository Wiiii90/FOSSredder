/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisTransactionsPanel.qml
 * @brief Provides QML tests for AnalysisTransactionsPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnalysisTransactionsPanelTests"
    when: windowShown
    width: 960
    height: 640

    property var theme: QtObject {
        property int formLabelWidth: 120
        property int formFieldWidth: 220
        property int spacingSmall: 6
        property int spacing: 8
        property int radius: 3
        property int borderWidthThin: 1
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textMuted: "#666666"
        property var analysis: QtObject {
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

    property var selectedIds: []
    property string calcName: ""
    property string calcPercent: ""
    property int applyCalls: 0
    property var analysisViewModel: QtObject {
        property string calcName: testCase.calcName
        property string calcPercentText: testCase.calcPercent
        property var selectedAdjustmentTxIds: testCase.selectedIds
        property var previewTransactionRows: [
            { id: "tx-1", statementName: "S1", transactionName: "Rent", date: "2026-01-01", valuta: "2026-01-02", actorName: "Alice", contractName: "Lease", contractType: "lease", propertiesLabel: "Lot", amountText: "100.00" }
        ]
        property string previewStatementCountText: "Statements: 1"
        property string previewTransactionCountText: "Transactions: 1"
        property string previewAmountSumText: "Amount sum: 100.00"
        function isAdjustmentTransactionSelected(id) { return testCase.selectedIds.indexOf(id) !== -1 }
        function setAdjustmentTransactionSelected(id, selected) { testCase.selectedIds = selected ? [id] : [] }
        function applySelectedCalc() { testCase.applyCalls += 1 }
    }

    Component {
        id: panelComponent
        Analysis.AnalysisTransactionsPanel {
            width: 960
            height: 640
            theme: testCase.theme
            analysisViewModel: testCase.analysisViewModel
        }
    }

    function createPanel() {
        const panel = createTemporaryObject(panelComponent, testCase)
        panel.visible = true
        return panel
    }

    function init() {
        selectedIds = []
        calcName = ""
        calcPercent = ""
        analysisViewModel.calcName = ""
        analysisViewModel.calcPercentText = ""
        analysisViewModel.selectedAdjustmentTxIds = []
        applyCalls = 0
    }

    function test_ANL_TP_001_transactionSelectionAndCalcSignals() {
        const panel = createPanel()
        tryVerify(function() { return Lookup.findObject(panel, "analysisTransactionSelectionCheckBox") !== null })
        const txCheck = TestSupport.findRequired(Lookup, panel, "analysisTransactionSelectionCheckBox")
        const calcNameField = TestSupport.findRequired(Lookup, panel, "analysisCalcNameField")
        const calcPercentField = TestSupport.findRequired(Lookup, panel, "analysisCalcPercentField")
        const applyCalcButton = TestSupport.findRequired(Lookup, panel, "analysisApplyCalcButton")

        txCheck.checked = true
        txCheck.toggled()
        compare(selectedIds.length, 1)
        compare(selectedIds[0], "tx-1")

        calcNameField.text = "VAT"
        calcPercentField.text = "19"
        compare(analysisViewModel.calcName, "VAT")
        compare(analysisViewModel.calcPercentText, "19")

        applyCalcButton.clicked()
        compare(applyCalls, 1)
    }
}
