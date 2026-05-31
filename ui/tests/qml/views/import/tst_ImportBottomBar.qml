/**
 * @file ui/tests/qml/views/import/tst_ImportBottomBar.qml
 * @brief Provides QML tests for ImportBottomBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ImportBottomBarTests"
    when: windowShown
    width: 900
    height: 120

    property var theme: QtObject {
        property int viewActionButtonWidth: 120
        property int viewNavigationButtonWidth: 42
    }

    property var importState: QtObject {
        property bool hasDraftNavigation: false
        property bool canClear: false
        property bool canCancel: false
        property bool canCancelAll: false
        property bool canPause: false
        property bool canStart: false
        property bool importRunning: false
        property string pauseText: "Pause"
        property int startCalls: 0
        property int clearCalls: 0
        property int cancelCalls: 0
        property int cancelAllCalls: 0
        property int pauseCalls: 0
        property int previousDraftCalls: 0
        property int nextDraftCalls: 0
        function startImport() { startCalls += 1 }
        function resetStatus() { clearCalls += 1 }
        function cancelImport() { cancelCalls += 1 }
        function cancelAllImports() { cancelAllCalls += 1 }
        function togglePause() { pauseCalls += 1 }
        function openPreviousDraft() { previousDraftCalls += 1 }
        function openNextDraft() { nextDraftCalls += 1 }
    }

    Component {
        id: bottomBarComponent
        Import.ImportBottomBar {
            width: testCase.width
            theme: testCase.theme
            importState: testCase.importState
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createBar() {
        return createTemporaryObject(bottomBarComponent, testCase)
    }

    function init() {
        importState.hasDraftNavigation = false
        importState.canClear = false
        importState.canCancel = false
        importState.canCancelAll = false
        importState.canPause = false
        importState.canStart = false
        importState.importRunning = false
        importState.startCalls = 0
        importState.clearCalls = 0
        importState.cancelCalls = 0
        importState.cancelAllCalls = 0
        importState.pauseCalls = 0
        importState.previousDraftCalls = 0
        importState.nextDraftCalls = 0
    }

    function test_IMP_V_002_startActionDelegatesToImportState() {
        importState.canStart = true
        const bar = createBar()

        findRequired(bar, "importStartButton").clicked()

        compare(importState.startCalls, 1)
    }

    function test_IMP_V_003_clearActionDelegatesToImportState() {
        importState.canClear = true
        const bar = createBar()

        findRequired(bar, "importClearButton").clicked()

        compare(importState.clearCalls, 1)
    }

    function test_IMP_V_004_runningActionsDelegateToImportState() {
        importState.importRunning = true
        importState.canCancel = true
        importState.canCancelAll = true
        importState.canPause = true
        const bar = createBar()

        findRequired(bar, "importCancelButton").clicked()
        findRequired(bar, "importCancelAllButton").clicked()
        findRequired(bar, "importPauseButton").clicked()

        compare(importState.cancelCalls, 1)
        compare(importState.cancelAllCalls, 1)
        compare(importState.pauseCalls, 1)
    }

    function test_IMP_V_009_draftNavigationDelegatesToImportState() {
        importState.hasDraftNavigation = true
        const bar = createBar()

        findRequired(bar, "importPreviousDraftButton").clicked()
        findRequired(bar, "importNextDraftButton").clicked()

        compare(importState.previousDraftCalls, 1)
        compare(importState.nextDraftCalls, 1)
    }
}
