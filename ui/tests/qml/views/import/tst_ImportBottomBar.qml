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

    property var importViewModel: QtObject {
        property bool hasDraftNavigation: false
        property bool canClearImport: false
        property bool canCancel: false
        property int queuedCount: 0
        property bool canPause: false
        property bool canStart: false
        property bool importRunning: false
        property bool importPaused: false
        property string pauseText: "Pause"
        property int startCalls: 0
        property int clearCalls: 0
        property int cancelCalls: 0
        property int cancelAllCalls: 0
        property int pauseCalls: 0
        property int resumeCalls: 0
        property int previousDraftCalls: 0
        property int nextDraftCalls: 0
        function startImport() { startCalls += 1 }
        function clearImport() { clearCalls += 1 }
        function cancelCurrentImport() { cancelCalls += 1 }
        function cancelAllImports() { cancelAllCalls += 1 }
        function pauseImport() { pauseCalls += 1 }
        function resumeImport() { resumeCalls += 1 }
        function selectPreviousDraft() { previousDraftCalls += 1 }
        function selectNextDraft() { nextDraftCalls += 1 }
    }

    Component {
        id: bottomBarComponent
        Import.ImportBottomBar {
            width: testCase.width
            theme: testCase.theme
            importViewModel: testCase.importViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createBar() {
        return createTemporaryObject(bottomBarComponent, testCase)
    }

    function init() {
        importViewModel.hasDraftNavigation = false
        importViewModel.canClearImport = false
        importViewModel.canCancel = false
        importViewModel.queuedCount = 0
        importViewModel.canPause = false
        importViewModel.canStart = false
        importViewModel.importRunning = false
        importViewModel.importPaused = false
        importViewModel.startCalls = 0
        importViewModel.clearCalls = 0
        importViewModel.cancelCalls = 0
        importViewModel.cancelAllCalls = 0
        importViewModel.pauseCalls = 0
        importViewModel.resumeCalls = 0
        importViewModel.previousDraftCalls = 0
        importViewModel.nextDraftCalls = 0
    }

    function test_IMP_V_002_startActionDelegatesToImportState() {
        importViewModel.canStart = true
        const bar = createBar()

        findRequired(bar, "importStartButton").clicked()

        compare(importViewModel.startCalls, 1)
    }

    function test_IMP_V_003_clearActionDelegatesToImportState() {
        importViewModel.canClearImport = true
        const bar = createBar()

        findRequired(bar, "importClearButton").clicked()

        compare(importViewModel.clearCalls, 1)
    }

    function test_IMP_V_004_runningActionsDelegateToImportState() {
        importViewModel.importRunning = true
        importViewModel.importPaused = false
        importViewModel.canCancel = true
        importViewModel.queuedCount = 1
        importViewModel.canPause = true
        const bar = createBar()

        findRequired(bar, "importCancelButton").clicked()
        findRequired(bar, "importCancelAllButton").clicked()
        findRequired(bar, "importPauseButton").clicked()

        compare(importViewModel.cancelCalls, 1)
        compare(importViewModel.cancelAllCalls, 1)
        compare(importViewModel.pauseCalls, 1)
        compare(importViewModel.resumeCalls, 0)
    }

    function test_IMP_V_009_draftNavigationDelegatesToImportState() {
        importViewModel.hasDraftNavigation = true
        const bar = createBar()

        findRequired(bar, "importPreviousDraftButton").clicked()
        findRequired(bar, "importNextDraftButton").clicked()

        compare(importViewModel.previousDraftCalls, 1)
        compare(importViewModel.nextDraftCalls, 1)
    }
}
