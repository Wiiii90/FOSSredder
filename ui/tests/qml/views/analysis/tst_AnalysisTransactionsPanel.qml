/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisTransactionsPanel.qml
 * @brief Provides QML tests for AnalysisTransactionsPanel behavior.
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
    name: "AnalysisTransactionsPanelTests"
    when: windowShown
    width: 960
    height: 640

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var selectedIds: []
    property string adjustmentName: ""
    property string adjustmentPercent: ""
    property int applyCalls: 0
    property var analysisViewModel: QtObject {
        property string adjustmentName: testCase.adjustmentName
        property string adjustmentPercentText: testCase.adjustmentPercent
        property var selectedAdjustmentTxIds: testCase.selectedIds
        property var previewTransactionRows: [
            { id: "tx-1", statementName: "S1", transactionName: "Rent", date: "2026-01-01", valuta: "2026-01-02", actorName: "Alice", contractName: "Lease", contractType: "lease", propertiesLabel: "Lot", amountText: "100.00" }
        ]
        property string previewStatementCountText: "Statements: 1"
        property string previewTransactionCountText: "Transactions: 1"
        property string previewAmountSumText: "Amount sum: 100.00"
        function setAdjustmentTransactionSelected(id, selected) { testCase.selectedIds = selected ? [id] : [] }
        function applySelectedAdjustment() { testCase.applyCalls += 1 }
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
        adjustmentName = ""
        adjustmentPercent = ""
        analysisViewModel.adjustmentName = ""
        analysisViewModel.adjustmentPercentText = ""
        analysisViewModel.selectedAdjustmentTxIds = []
        applyCalls = 0
    }

    function test_ANL_TP_001_transactionSelectionAndAdjustmentSignals() {
        const panel = createPanel()
        tryVerify(function() { return Lookup.findObject(panel, "analysisTransactionSelectionCheckBox") !== null })
        const txCheck = TestSupport.findRequired(Lookup, panel, "analysisTransactionSelectionCheckBox")
        const adjustmentNameField = TestSupport.findRequired(Lookup, panel, "analysisAdjustmentNameField")
        const adjustmentPercentField = TestSupport.findRequired(Lookup, panel, "analysisAdjustmentPercentField")
        const applyAdjustmentButton = TestSupport.findRequired(Lookup, panel, "analysisApplyAdjustmentButton")

        txCheck.checked = true
        txCheck.toggled()
        compare(selectedIds.length, 1)
        compare(selectedIds[0], "tx-1")

        adjustmentNameField.text = "VAT"
        adjustmentPercentField.text = "19"
        compare(analysisViewModel.adjustmentName, "VAT")
        compare(analysisViewModel.adjustmentPercentText, "19")

        applyAdjustmentButton.clicked()
        compare(applyCalls, 1)
    }
}
