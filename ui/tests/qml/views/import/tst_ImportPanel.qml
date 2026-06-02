/**
 * @file ui/tests/qml/views/import/tst_ImportPanel.qml
 * @brief Provides QML tests for ImportPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ImportPanelTests"
    when: windowShown
    width: 900
    height: 360

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacingMedium: 8
        property int formLabelWidth: 120
        property int viewActionButtonWidth: 120
        property int panelPadding: 12
        property int radius: 3
        property int borderWidthThin: 1
        property int controlHeight: 32
        property color border: "#cccccc"
        property color borderSoft: "#cccccc"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    property var importViewModel: QtObject {
        property bool hasImportWorkflow: true
        property bool importRunning: false
        property string manualPathText: ""
        property int queuedCount: 0
        property var importFiles: []
        property int browseCalls: 0
        property int commitManualCalls: 0
        function browseImportPdf() { browseCalls += 1 }
        function addSelectedImportFiles() { commitManualCalls += 1 }
    }

    Component {
        id: panelComponent
        Import.ImportPanel {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            importViewModel: testCase.importViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createPanel() {
        return createTemporaryObject(panelComponent, testCase)
    }

    function init() {
        importViewModel.hasImportWorkflow = true
        importViewModel.importRunning = false
        importViewModel.manualPathText = ""
        importViewModel.queuedCount = 0
        importViewModel.importFiles = []
        importViewModel.browseCalls = 0
        importViewModel.commitManualCalls = 0
    }

    function test_IMP_P_001_manualPathEditingAndAddDelegatesToImportState() {
        const panel = createPanel()
        const field = findRequired(panel, "importManualPathField")

        field.text = "P:/imports/statement.pdf"
        field.textEdited()
        findRequired(panel, "importAddFileButton").clicked()

        compare(importViewModel.manualPathText, "P:/imports/statement.pdf")
        compare(importViewModel.commitManualCalls, 1)
    }

    function test_IMP_P_002_browseDelegatesToImportState() {
        const panel = createPanel()

        findRequired(panel, "importBrowseFileButton").clicked()

        compare(importViewModel.browseCalls, 1)
    }
}
