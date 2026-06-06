/**
 * @file ui/tests/qml/views/annual/tst_AnnualView.qml
 * @brief Provides QML composition tests for AnnualView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
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

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

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
