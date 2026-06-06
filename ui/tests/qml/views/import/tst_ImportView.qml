/**
 * @file ui/tests/qml/views/import/tst_ImportView.qml
 * @brief Provides QML composition tests for ImportView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ImportViewTests"
    when: windowShown
    width: 960
    height: 640

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var importViewModel: QtObject {
        property var importWorkflow: null
        property var navigation: null
        property var workspace: null
        property int contentIndex: 0
        property bool hasImportWorkflow: true
        property bool hasDraft: false
        property bool hasDraftNavigation: false
        property bool canClearImport: false
        property bool canCancel: false
        property bool canPause: false
        property bool canStart: false
        property bool importRunning: false
        property bool importPaused: false
        property string pauseText: "Pause"
        property string manualPathText: ""
        property string progressText: ""
        property bool progressHasError: false
        property real progressValue: 0
        property int queuedCount: 0
        property var importFiles: []
        property string importFileSummary: ""
        function initializeImportView() {}
        function browseImportPdf() {}
        function addSelectedImportFiles() {}
        function selectPreviousDraft() {}
        function selectNextDraft() {}
        function clearImport() {}
        function cancelCurrentImport() {}
        function cancelQueuedImports() {}
        function pauseImport() {}
        function resumeImport() {}
        function startImport() {}
    }

    Component {
        id: importViewComponent
        Import.ImportView {
            width: testCase.width
            height: testCase.height
            importViewModel: testCase.importViewModel
            theme: testCase.theme
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_IMP_V_001_importViewComposesContentStack() {
        const view = createTemporaryObject(importViewComponent, testCase)

        verify(findRequired(view, "importContentStack") !== null)
    }
}
