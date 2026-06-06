/**
 * @file ui/tests/qml/views/import/tst_ImportHomeView.qml
 * @brief Provides QML composition tests for ImportHomeView.
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
    name: "ImportHomeViewTests"
    when: windowShown
    width: 960
    height: 640

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var importViewModel: QtObject {
        property bool hasImportWorkflow: true
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
        id: homeComponent
        Import.ImportHomeView {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            importViewModel: testCase.importViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_IMP_H_001_composesImportOverviewSections() {
        const home = createTemporaryObject(homeComponent, testCase)

        verify(findRequired(home, "importManualPathField") !== null)
        verify(findRequired(home, "importProgressBar") !== null)
        verify(findRequired(home, "importStartButton") !== null)
    }
}
