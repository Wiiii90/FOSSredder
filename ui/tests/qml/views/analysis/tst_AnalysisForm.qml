/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisForm.qml
 * @brief Provides QML tests for AnalysisForm presentation bindings.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnalysisFormTests"
    when: windowShown
    width: 960
    height: 640

    Item {
        id: sceneRoot
        width: testCase.width
        height: testCase.height
        visible: true
    }

    property var theme: QtObject {
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
        property string name: ""
        property int mainTypeIndex: 0
        property int plotSubtypeIndex: 0
        property var plotTypeOptions: [{ value: "pie", label: "Pie chart" }, { value: "histogram", label: "Histogram" }]
        property var exportFormatOptions: [{ value: "png", label: "PNG" }, { value: "jpg", label: "JPG" }]
        property string exportFormat: "png"
        property bool includeCalcAdjustments: true
        property string exportStateJson: "{}"
        property bool filterEditMode: true
        property int filterWorkspaceIndex: 0
        property int dateFieldIndex: 0
        property int dateModeIndex: 0
        property string yearValue: "2025"
        property string dateFromValue: ""
        property string dateToValue: ""
        property var propertyFilterRows: [{ id: "property-1", name: "Lot" }, { id: "unassigned", name: "Unassigned" }]
        property var contractTypeRows: [{ value: "lease", label: "lease" }, { value: "unassigned", label: "Unassigned" }]
        property int exportFormatIndex: 0
        property bool currentResultIsTable: currentResultType === "tab"
        property var selectedPropertyIds: ["property-1", "unassigned"]
        property var selectedContractTypes: ["lease", "unassigned"]
        property string allocatableMode: "all"
        property var previewTransactions: [{ id: "tx-1", transactionName: "Rent", amount: 10.0 }]
        property var previewTransactionRows: [{ id: "tx-1", statementName: "S1", transactionName: "Rent", date: "2026-01-01", valuta: "2026-01-02", actorName: "Alice", contractName: "Lease", contractType: "lease", propertiesLabel: "Lot", amountText: "10.00" }]
        property var previewMetrics: ({ statementCount: 1, transactionCount: 1, amountSum: 10.0 })
        property string previewStatementCountText: "Statements: 1"
        property string previewTransactionCountText: "Transactions: 1"
        property string previewAmountSumText: "Amount sum: 10.00"
        property var selectedAdjustmentTxIds: []
        property var adjustmentAmountsById: ({})
        property string calcName: ""
        property string calcPercentText: ""
        property string pendingAdjustmentsJson: "{}"
        property string currentResultType: "plot"
        property string renderedPreviewSource: ""
        property var tableContractTypes: []
        property var tablePropertyRows: []
        property real tableGrandTotal: 0.0

        function applySelectedCalc() {}
        function setExportFormatIndex(index) { exportFormatIndex = index; exportFormat = exportFormatOptions[index].value }
        function isPropertySelected(id) { return selectedPropertyIds.indexOf(id) !== -1 }
        function setPropertySelected(id, selected) {
            let next = selectedPropertyIds.slice()
            const existing = next.indexOf(id)
            if (selected && existing === -1)
                next.push(id)
            if (!selected && existing !== -1)
                next.splice(existing, 1)
            selectedPropertyIds = next
        }
        function selectAllProperties() { selectedPropertyIds = ["property-1", "unassigned"] }
        function selectUnassignedProperties() { selectedPropertyIds = ["unassigned"] }
        function isContractTypeSelected(type) { return selectedContractTypes.indexOf(type) !== -1 }
        function setContractTypeSelected(type, selected) {
            let next = selectedContractTypes.slice()
            const existing = next.indexOf(type)
            if (selected && existing === -1)
                next.push(type)
            if (!selected && existing !== -1)
                next.splice(existing, 1)
            selectedContractTypes = next
        }
        function selectAllContractTypes() { selectedContractTypes = ["lease", "unassigned"] }
        function selectUnassignedContractTypes() { selectedContractTypes = ["unassigned"] }
        function isAdjustmentTransactionSelected(id) { return selectedAdjustmentTxIds.indexOf(id) !== -1 }
        function setAdjustmentTransactionSelected(id, selected) { selectedAdjustmentTxIds = selected ? [id] : [] }
        function setAllocatableModeIndex(index) { allocatableMode = index === 1 ? "allocatable" : (index === 2 ? "non-allocatable" : "all") }
    }

    Component {
        id: formComponent

        Analysis.AnalysisForm {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            analysisState: testCase.analysisState
        }
    }

    function createForm() {
        const form = createTemporaryObject(formComponent, sceneRoot)
        form.visible = true
        return form
    }

    function init() {
        analysisState.isEdit = false
        analysisState.name = ""
        analysisState.mainTypeIndex = 0
        analysisState.selectedPropertyIds = ["property-1", "unassigned"]
        analysisState.selectedContractTypes = ["lease", "unassigned"]
    }

    function test_ANL_F_001_nameFieldWritesState() {
        const form = createForm()

        TestSupport.findRequired(Lookup, form, "analysisNameField").text = "Analysis"

        compare(analysisState.name, "Analysis")
    }

    function test_ANL_F_002_filterPanelsForwardSelectionToState() {
        const form = createForm()

        const checkBox = TestSupport.findRequired(Lookup, form, "analysisPropertyFilterCheckBox")
        checkBox.checked = false
        checkBox.clicked()

        compare(analysisState.selectedPropertyIds.length, 1)
        compare(analysisState.selectedPropertyIds[0], "unassigned")

        TestSupport.findRequired(Lookup, form, "analysisPropertyFilterAllButton").clicked()
        compare(analysisState.selectedPropertyIds.length, 2)
        compare(analysisState.selectedPropertyIds[0], "property-1")

        TestSupport.findRequired(Lookup, form, "analysisPropertyFilterUnassignedButton").clicked()
        compare(analysisState.selectedPropertyIds.length, 1)
        compare(analysisState.selectedPropertyIds[0], "unassigned")

        TestSupport.findRequired(Lookup, form, "analysisContractTypeFilterUnassignedButton").clicked()
        compare(analysisState.selectedContractTypes.length, 1)
        compare(analysisState.selectedContractTypes[0], "unassigned")

        TestSupport.findRequired(Lookup, form, "analysisContractTypeFilterAllButton").clicked()
        compare(analysisState.selectedContractTypes.length, 2)
        compare(analysisState.selectedContractTypes[0], "lease")
    }

    function test_ANL_F_003_editModeShowsResultPanel() {
        analysisState.isEdit = true
        analysisState.renderedPreviewSource = "data:image/svg+xml,%3Csvg width='1' height='1' viewBox='0 0 1 1'%3E%3Crect width='1' height='1' fill='red'/%3E%3C/svg%3E"
        const form = createForm()

        verify(TestSupport.findRequired(Lookup, form, "analysisPreviewImage") !== null)
    }

    function test_ANL_F_004_includeCalcToggleWritesState() {
        analysisState.isEdit = true
        analysisState.includeCalcAdjustments = true
        const form = createForm()
        compare(form.isEdit, true)

        const includeCalcMouseArea = TestSupport.findRequired(Lookup, form, "analysisIncludeCalcAdjustmentsMouseArea")
        includeCalcMouseArea.clicked(null)

        compare(analysisState.includeCalcAdjustments, false)
        const includeCalcCheckBox = TestSupport.findRequired(Lookup, form, "analysisIncludeCalcAdjustmentsCheckBox")
        compare(includeCalcCheckBox.checked, false)
    }
}
