/**
 * @file ui/tests/qml/views/import/tst_ImportHomeView.qml
 * @brief Provides QML composition tests for ImportHomeView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ImportHomeViewTests"
    when: windowShown
    width: 960
    height: 640

    property var theme: QtObject {
        property int spacing: 8
        property int spacingSmall: 6
        property int spacingMedium: 8
        property int formLabelWidth: 120
        property int viewActionButtonWidth: 120
        property int viewNavigationButtonWidth: 42
        property int viewCompactActionButtonSize: 28
        property int radius: 3
        property int borderWidthThin: 1
        property int panelPadding: 12
        property int controlHeight: 32
        property color border: "#cccccc"
        property color borderSoft: "#cccccc"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color danger: "#aa0000"
        property color success: "#008800"
        property color successStrong: "#008800"
        property color warning: "#aa8800"
    }

    property var importState: QtObject {
        property bool hasImportWorkflow: true
        property bool hasDraftNavigation: false
        property bool canClear: false
        property bool canCancel: false
        property bool canCancelAll: false
        property bool canPause: false
        property bool canStart: false
        property bool importRunning: false
        property string pauseText: "Pause"
        property string manualPathText: ""
        property string progressText: ""
        property bool progressHasError: false
        property real progressValue: 0
        property int queuedCount: 0
        property var importFiles: []
        function browseImportPdf() {}
        function commitManualImportFiles() {}
        function openPreviousDraft() {}
        function openNextDraft() {}
        function resetStatus() {}
        function cancelImport() {}
        function cancelAllImports() {}
        function togglePause() {}
        function startImport() {}
    }

    Component {
        id: homeComponent
        Import.ImportHomeView {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            importState: testCase.importState
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
