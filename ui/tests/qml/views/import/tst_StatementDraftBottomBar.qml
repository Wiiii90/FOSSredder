/**
 * @file ui/tests/qml/views/import/tst_StatementDraftBottomBar.qml
 * @brief Provides QML tests for StatementDraftBottomBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "StatementDraftBottomBarTests"
    when: windowShown
    width: 900
    height: 120

    property var theme: QtObject {
        property int viewActionButtonWidth: 120
        property int viewNavigationButtonWidth: 42
    }

    property var statementViewModel: QtObject {
        property bool hasDraft: true
        property bool canSelectPreviousTransactionDraft: true
        property bool canSelectNextTransactionDraft: true
        property int returnCalls: 0
        property int discardCalls: 0
        property int finalizeCalls: 0
        property int previousTransactionCalls: 0
        property int nextTransactionCalls: 0
        function returnToImport() { returnCalls += 1 }
        function discard() { discardCalls += 1 }
        function finalize() { finalizeCalls += 1 }
        function selectPreviousTransactionDraft() { previousTransactionCalls += 1 }
        function selectNextTransactionDraft() { nextTransactionCalls += 1 }
    }

    property var importViewModel: QtObject {
        property bool hasDraftNavigation: true
        property int previousDraftCalls: 0
        property int nextDraftCalls: 0
        function selectPreviousDraft() { previousDraftCalls += 1 }
        function selectNextDraft() { nextDraftCalls += 1 }
    }

    Component {
        id: barComponent
        Import.StatementDraftBottomBar {
            width: testCase.width
            theme: testCase.theme
            importViewModel: testCase.importViewModel
            statementViewModel: testCase.statementViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createBar() {
        return createTemporaryObject(barComponent, testCase)
    }

    function init() {
        statementViewModel.hasDraft = true
        statementViewModel.canSelectPreviousTransactionDraft = true
        statementViewModel.canSelectNextTransactionDraft = true
        importViewModel.hasDraftNavigation = true
        statementViewModel.returnCalls = 0
        statementViewModel.discardCalls = 0
        statementViewModel.finalizeCalls = 0
        statementViewModel.previousTransactionCalls = 0
        statementViewModel.nextTransactionCalls = 0
        importViewModel.previousDraftCalls = 0
        importViewModel.nextDraftCalls = 0
    }

    function test_IMP_D_001_statementBottomBarReturnDelegatesToState() {
        const bar = createBar()

        findRequired(bar, "statementDraftReturnButton").clicked()

        compare(statementViewModel.returnCalls, 1)
    }

    function test_IMP_D_002_statementBottomBarDiscardDelegatesToState() {
        const bar = createBar()

        findRequired(bar, "statementDraftDiscardButton").clicked()

        compare(statementViewModel.discardCalls, 1)
    }

    function test_IMP_D_003_statementBottomBarFinalizeDelegatesToState() {
        const bar = createBar()

        findRequired(bar, "statementDraftFinalizeButton").clicked()

        compare(statementViewModel.finalizeCalls, 1)
    }

    function test_IMP_D_004_statementBottomBarDisablesLifecycleActionsWithoutDraft() {
        statementViewModel.hasDraft = false
        const bar = createBar()

        compare(findRequired(bar, "statementDraftReturnButton").enabled, false)
        compare(findRequired(bar, "statementDraftDiscardButton").enabled, false)
        compare(findRequired(bar, "statementDraftFinalizeButton").enabled, false)
    }

    function test_IMP_D_005_statementBottomBarDelegatesTransactionAndDraftNavigation() {
        const bar = createBar()

        findRequired(bar, "statementDraftPrevTransactionButton").clicked()
        findRequired(bar, "statementDraftNextTransactionButton").clicked()
        findRequired(bar, "statementDraftPrevPageButton").clicked()
        findRequired(bar, "statementDraftNextPageButton").clicked()

        compare(statementViewModel.previousTransactionCalls, 1)
        compare(statementViewModel.nextTransactionCalls, 1)
        compare(importViewModel.previousDraftCalls, 1)
        compare(importViewModel.nextDraftCalls, 1)
    }

    function test_IMP_D_007_statementBottomBarDisablesUnavailableNavigation() {
        statementViewModel.canSelectPreviousTransactionDraft = false
        statementViewModel.canSelectNextTransactionDraft = false
        importViewModel.hasDraftNavigation = false
        const bar = createBar()

        compare(findRequired(bar, "statementDraftPrevTransactionButton").enabled, false)
        compare(findRequired(bar, "statementDraftNextTransactionButton").enabled, false)
        compare(findRequired(bar, "statementDraftPrevPageButton").enabled, false)
        compare(findRequired(bar, "statementDraftNextPageButton").enabled, false)
    }
}
