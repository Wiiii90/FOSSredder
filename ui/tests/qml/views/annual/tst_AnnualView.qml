/**
 * @file ui/tests/qml/views/annual/tst_AnnualView.qml
 * @brief Provides QML composition tests for AnnualView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Annual 1.0 as Annual

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnnualViewTests"
    when: windowShown
    width: 960
    height: 640

    property var annualViewModel: QtObject {
        property bool isEdit: false
        property bool canSubmit: true
        property bool hasRows: true
        property bool hasChanges: true
        property string name: ""
        property int year: 2026
        property int contentIndex: 0
        property var annualRows: []
        property string selectedAnnualId: ""
        property var availableAnalysisRows: []
        property var assignedAnalysisRows: []
        property var annualTransactions: []
        property var transactionSections: []
        property var verificationRows: []
        property string statusSummaryText: "Neutral: 0, Unverified: 0, Verified: 0, Completed: 0"
        property int refreshCalls: 0
        property int createCalls: 0
        function refreshFromSelection() { refreshCalls += 1 }
        function submitCreate() { createCalls += 1 }
        function resetCreateState() {}
        function submitUpdate() {}
        function deleteCurrent() {}
        function navigate(delta) {}
        function toggleContent() {}
        function addAvailableAnalysisAtIndex(index) {}
        function removeAnalysis(id) {}
        function setAnalysisExportFormat(id, exportFormat) {}
        function toggleTransactionSection(key) {}
        function stepYear(delta) { year += delta }
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int viewFormSpacing: 8
        property int formLabelWidth: 120
        property int formFieldWidth: 200
        property int spacingSmall: 6
        property int spacing: 8
        property int viewSelectionPanelMinHeight: 160
        property int viewSelectionPanelPreferredHeight: 220
        property int controlHeight: 32
        property int viewCompactActionButtonSize: 28
        property int viewCompactActionButtonSizeSmall: 32
        property int viewActionButtonWidth: 120
        property int viewNavigationButtonWidth: 42
        property int radius: 3
        property int borderWidthThin: 1
        property color borderSoft: "#cccccc"
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color danger: "#b0302f"
        property color success: "#0a7f2e"
        property color warning: "#a86d00"
        property color info: "#1a73b8"
        property var annual: ({
            transactions: {
                tableMinWidth: 720,
                dateColumnWidth: 110,
                amountColumnWidth: 90,
                allocatableColumnWidth: 130,
                typeColumnWidth: 120,
                statusColumnWidth: 100
            }
        })
    }

    Component {
        id: annualViewComponent
        Annual.AnnualView {
            width: 960
            height: 640
            annualViewModel: testCase.annualViewModel
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(annualViewComponent, testCase)
    }

    function init() {
        annualViewModel.refreshCalls = 0
        annualViewModel.createCalls = 0
    }

    function test_ANN_V_001_mountsFormAndBottomBarWithAnnualState() {
        const view = createView()
        verify(TestSupport.findRequired(Lookup, view, "annualNameField") !== null)
        verify(TestSupport.findRequired(Lookup, view, "annualCreateButton") !== null)
    }

    function test_ANN_V_002_bottomBarCommandUsesInjectedAnnualState() {
        const view = createView()
        TestSupport.findRequired(Lookup, view, "annualCreateButton").clicked()
        compare(annualViewModel.createCalls, 1)
    }
}
