/**
 * @file ui/tests/qml/views/export/tst_ExportForm.qml
 * @brief Provides QML tests for ExportForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Export 1.0 as Export

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ExportFormTests"
    when: windowShown
    width: 960
    height: 640

    property var exportViewModel: QtObject {
        property string targetDirectory: "test:///export/default"
        property int packageFormatIndex: 0
        property int browseCalls: 0
        function browseDirectory() { browseCalls += 1 }
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: exportFormComponent
        Export.ExportForm {
            width: 700
            theme: testCase.theme
            exportViewModel: testCase.exportViewModel
        }
    }

    function createForm() {
        return createTemporaryObject(exportFormComponent, testCase)
    }

    function init() {
        exportViewModel.targetDirectory = "test:///export/default"
        exportViewModel.packageFormatIndex = 0
        exportViewModel.browseCalls = 0
    }

    function test_EXP_F_001_targetDirectoryFieldUpdatesExportState() {
        const form = createForm()
        const targetField = TestSupport.findRequired(Lookup, form, "exportTargetDirectoryField")

        targetField.text = "test:///export/target"

        compare(exportViewModel.targetDirectory, "test:///export/target")
    }

    function test_EXP_F_002_archiveFormatDropdownUpdatesExportState() {
        const form = createForm()
        const archiveCombo = TestSupport.findRequired(Lookup, form, "exportArchiveFormatComboBox")

        archiveCombo.currentIndex = 1

        compare(exportViewModel.packageFormatIndex, 1)
    }

    function test_EXP_F_003_browseButtonDelegatesToExportState() {
        const form = createForm()
        const browseButton = TestSupport.findRequired(Lookup, form, "exportBrowseDirectoryButton")

        browseButton.clicked()

        compare(exportViewModel.browseCalls, 1)
    }
}
