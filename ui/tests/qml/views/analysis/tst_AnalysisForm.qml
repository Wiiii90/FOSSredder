/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisForm.qml
 * @brief Provides QML tests for AnalysisForm presentation bindings.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common"

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

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var analysisViewModel: QtObject {
        property bool isEdit: false
        property string name: ""
        property int mainTypeIndex: 0
        property int plotSubtypeIndex: 0
        property var plotTypeOptions: [{ value: "pie", label: "Pie chart" }, { value: "histogram", label: "Histogram" }]
        property var exportFormatOptions: [{ value: "png", label: "PNG" }, { value: "jpg", label: "JPG" }]
        property string exportFormat: "png"
        property bool includeAdjustments: true
        property bool filterEditMode: true
        property int filterContentIndex: 0
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
        property var previewTransactionRows: [{ id: "tx-1", statementName: "S1", transactionName: "Rent", date: "2026-01-01", valuta: "2026-01-02", actorName: "Alice", contractName: "Lease", contractType: "lease", propertiesLabel: "Lot", amountText: "10.00" }]
        property string previewStatementCountText: "Statements: 1"
        property string previewTransactionCountText: "Transactions: 1"
        property string previewAmountSumText: "Amount sum: 10.00"
        property var selectedAdjustmentTxIds: []
        property string adjustmentName: ""
        property string adjustmentPercentText: ""
        property string currentResultType: "plot"
        property string renderedPreviewSource: ""
        property var tableContractTypes: []
        property var tablePropertyRows: []
        property real tableGrandTotal: 0.0

        function applySelectedAdjustment() {}
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
        function selectNoProperties() { selectedPropertyIds = [] }
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
        function selectNoContractTypes() { selectedContractTypes = [] }
        function setAdjustmentTransactionSelected(id, selected) { selectedAdjustmentTxIds = selected ? [id] : [] }
        function setAllocatableModeIndex(index) { allocatableMode = index === 1 ? "allocatable" : (index === 2 ? "non-allocatable" : "all") }
    }

    Component {
        id: formComponent

        Analysis.AnalysisForm {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            analysisViewModel: testCase.analysisViewModel
        }
    }

    function createForm() {
        const form = createTemporaryObject(formComponent, sceneRoot)
        form.visible = true
        return form
    }

    function init() {
        analysisViewModel.isEdit = false
        analysisViewModel.name = ""
        analysisViewModel.mainTypeIndex = 0
        analysisViewModel.selectedPropertyIds = ["property-1", "unassigned"]
        analysisViewModel.selectedContractTypes = ["lease", "unassigned"]
    }

    function test_ANL_F_001_nameFieldWritesState() {
        const form = createForm()

        TestSupport.findRequired(Lookup, form, "analysisNameField").text = "Analysis"

        compare(analysisViewModel.name, "Analysis")
    }

    function test_ANL_F_002_filterPanelsForwardSelectionToState() {
        const form = createForm()

        const checkBox = TestSupport.findRequired(Lookup, form, "analysisPropertyFilterCheckBox")
        checkBox.checked = false
        checkBox.clicked()

        compare(analysisViewModel.selectedPropertyIds.length, 1)
        compare(analysisViewModel.selectedPropertyIds[0], "unassigned")

        TestSupport.findRequired(Lookup, form, "analysisPropertyFilterAllButton").clicked()
        compare(analysisViewModel.selectedPropertyIds.length, 2)
        compare(analysisViewModel.selectedPropertyIds[0], "property-1")

        TestSupport.findRequired(Lookup, form, "analysisPropertyFilterNoneButton").clicked()
        compare(analysisViewModel.selectedPropertyIds.length, 0)

        TestSupport.findRequired(Lookup, form, "analysisContractTypeFilterNoneButton").clicked()
        compare(analysisViewModel.selectedContractTypes.length, 0)

        TestSupport.findRequired(Lookup, form, "analysisContractTypeFilterAllButton").clicked()
        compare(analysisViewModel.selectedContractTypes.length, 2)
        compare(analysisViewModel.selectedContractTypes[0], "lease")
    }

    function test_ANL_F_003_editModeShowsResultPanel() {
        analysisViewModel.isEdit = true
        analysisViewModel.renderedPreviewSource = "data:image/svg+xml,%3Csvg width='1' height='1' viewBox='0 0 1 1'%3E%3Crect width='1' height='1' fill='red'/%3E%3C/svg%3E"
        const form = createForm()

        verify(TestSupport.findRequired(Lookup, form, "analysisPreviewImage") !== null)
    }

    function test_ANL_F_004_includeAdjustmentsToggleWritesState() {
        analysisViewModel.isEdit = true
        analysisViewModel.includeAdjustments = true
        const form = createForm()
        compare(form.isEdit, true)

        const includeAdjustmentsMouseArea = TestSupport.findRequired(Lookup, form, "analysisIncludeAdjustmentsMouseArea")
        includeAdjustmentsMouseArea.clicked(null)

        compare(analysisViewModel.includeAdjustments, false)
        const includeAdjustmentsCheckBox = TestSupport.findRequired(Lookup, form, "analysisIncludeAdjustmentsCheckBox")
        compare(includeAdjustmentsCheckBox.checked, false)
    }
}
