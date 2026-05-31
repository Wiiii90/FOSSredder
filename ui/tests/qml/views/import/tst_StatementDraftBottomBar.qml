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

    property var statementState: QtObject {
        property bool hasDraft: true
        property bool canOpenPreviousTransaction: true
        property bool canOpenNextTransaction: true
        property bool canOpenPreviousDraft: true
        property bool canOpenNextDraft: true
        property int returnCalls: 0
        property int discardCalls: 0
        property int finalizeCalls: 0
        property int previousTransactionCalls: 0
        property int nextTransactionCalls: 0
        property int previousDraftCalls: 0
        property int nextDraftCalls: 0
        function returnToImport() { returnCalls += 1 }
        function discardDraft() { discardCalls += 1 }
        function finalizeDraft() { finalizeCalls += 1 }
        function openPreviousTransaction() { previousTransactionCalls += 1 }
        function openNextTransaction() { nextTransactionCalls += 1 }
        function openPreviousDraft() { previousDraftCalls += 1 }
        function openNextDraft() { nextDraftCalls += 1 }
    }

    Component {
        id: barComponent
        Import.StatementDraftBottomBar {
            width: testCase.width
            theme: testCase.theme
            statementState: testCase.statementState
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createBar() {
        return createTemporaryObject(barComponent, testCase)
    }

    function init() {
        statementState.hasDraft = true
        statementState.canOpenPreviousTransaction = true
        statementState.canOpenNextTransaction = true
        statementState.canOpenPreviousDraft = true
        statementState.canOpenNextDraft = true
        statementState.returnCalls = 0
        statementState.discardCalls = 0
        statementState.finalizeCalls = 0
        statementState.previousTransactionCalls = 0
        statementState.nextTransactionCalls = 0
        statementState.previousDraftCalls = 0
        statementState.nextDraftCalls = 0
    }

    function test_IMP_D_001_statementBottomBarReturnDelegatesToState() {
        const bar = createBar()

        findRequired(bar, "statementDraftReturnButton").clicked()

        compare(statementState.returnCalls, 1)
    }

    function test_IMP_D_002_statementBottomBarDiscardDelegatesToState() {
        const bar = createBar()

        findRequired(bar, "statementDraftDiscardButton").clicked()

        compare(statementState.discardCalls, 1)
    }

    function test_IMP_D_003_statementBottomBarFinalizeDelegatesToState() {
        const bar = createBar()

        findRequired(bar, "statementDraftFinalizeButton").clicked()

        compare(statementState.finalizeCalls, 1)
    }

    function test_IMP_D_004_statementBottomBarDisablesLifecycleActionsWithoutDraft() {
        statementState.hasDraft = false
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

        compare(statementState.previousTransactionCalls, 1)
        compare(statementState.nextTransactionCalls, 1)
        compare(statementState.previousDraftCalls, 1)
        compare(statementState.nextDraftCalls, 1)
    }

    function test_IMP_D_007_statementBottomBarDisablesUnavailableNavigation() {
        statementState.canOpenPreviousTransaction = false
        statementState.canOpenNextTransaction = false
        statementState.canOpenPreviousDraft = false
        statementState.canOpenNextDraft = false
        const bar = createBar()

        compare(findRequired(bar, "statementDraftPrevTransactionButton").enabled, false)
        compare(findRequired(bar, "statementDraftNextTransactionButton").enabled, false)
        compare(findRequired(bar, "statementDraftPrevPageButton").enabled, false)
        compare(findRequired(bar, "statementDraftNextPageButton").enabled, false)
    }
}
