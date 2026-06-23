/**
 * @file ui/tests/qml/views/annual/tst_AnnualForm.qml
 * @brief Provides QML tests for AnnualForm presentation bindings.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Annual 1.0 as Annual

import "../../common"

TestCase {
    id: testCase
    name: "AnnualFormTests"
    when: windowShown
    width: 960
    height: 640

    property var annualViewModel: QtObject {
        property bool isEdit: false
        property string name: ""
        property int year: 2026
        property int contentIndex: 0
        property var availableAnalysisRows: [
            { id: "analysis-1", display: "Analysis 1", typeLabel: "Table", exportFormatOptions: ["XLSX", "CSV"], exportFormatIndex: 0 }
        ]
        property var assignedAnalysisRows: []
        property var annualTransactions: [{ id: "tx-1" }]
        property var transactionSections: [
            { key: "deduplicated", title: "Included entries", expanded: true, visible: true, rows: [
                {
                    id: "tx-1",
                    name: "Transaction",
                    sourceNamesText: "",
                    bookingDate: "2026-01-01",
                    amountText: "10.00",
                    allocatableText: "Allocatable",
                    allocatable: true,
                    contractType: "rent",
                    contractTypeLabel: "rent",
                    statusText: "Neutral",
                    statusTone: "primary",
                    isMixedYear: false
                }
            ] }
        ]
        property var verificationRows: []
        property string statusSummaryText: "Neutral: 0, Unverified: 0, Verified: 0, Completed: 0"

        function addAvailableAnalysisAtIndex(index) {}
        function removeAnalysis(id) {}
        function setAnalysisExportFormat(id, exportFormat) {}
        function toggleTransactionSection(key) {}
        function stepYear(delta) { year += delta }
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: annualFormComponent
        Annual.AnnualForm {
            width: 960
            height: 640
            annualViewModel: testCase.annualViewModel
            theme: testCase.theme
        }
    }

    function createForm() {
        return createTemporaryObject(annualFormComponent, testCase)
    }

    function init() {
        annualViewModel.name = ""
        annualViewModel.year = 2026
        annualViewModel.contentIndex = 0
        annualViewModel.assignedAnalysisRows = []
        annualViewModel.availableAnalysisRows = [
            { id: "analysis-1", display: "Analysis 1", typeLabel: "Table", exportFormatOptions: ["XLSX", "CSV"], exportFormatIndex: 0 }
        ]
    }

    function test_ANN_F_001_nameAndYearControlsWriteAnnualState() {
        const form = createForm()
        const nameField = TestSupport.findRequired(Lookup, form, "annualNameField")
        const yearField = TestSupport.findRequired(Lookup, form, "annualYearField")

        nameField.text = "Annual 2026"
        compare(annualViewModel.name, "Annual 2026")

        TestSupport.findRequired(Lookup, form, "annualYearIncreaseButton").clicked()
        compare(annualViewModel.year, 2027)
    }

    function test_ANN_F_002_workspaceStackFollowsAnnualState() {
        const form = createForm()
        verify(TestSupport.findRequired(Lookup, form, "annualAddAnalysisComboBox") !== null)

        annualViewModel.contentIndex = 1
        wait(0)
        verify(Lookup.findObject(form, "annualTransactionsSectionToggle_deduplicated") !== null)
    }
}
