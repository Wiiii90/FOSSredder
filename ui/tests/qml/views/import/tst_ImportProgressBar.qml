/**
 * @file ui/tests/qml/views/import/tst_ImportProgressBar.qml
 * @brief Provides QML tests for ImportProgressBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Import 1.0 as Import

import "../../common"

TestCase {
    id: testCase
    name: "ImportProgressBarTests"
    when: windowShown
    width: 900
    height: 120

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var importViewModel: QtObject {
        property string progressText: ""
        property bool progressHasError: false
        property real progressValue: 0
    }

    Component {
        id: progressComponent
        Import.ImportProgressBar {
            width: testCase.width
            theme: testCase.theme
            importViewModel: testCase.importViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_IMP_PB_001_progressBindingUsesImportState() {
        importViewModel.progressValue = 0.42
        importViewModel.progressText = "Parsing page 4"
        const progress = createTemporaryObject(progressComponent, testCase)

        compare(findRequired(progress, "importProgressBar").value, 0.42)
    }
}
