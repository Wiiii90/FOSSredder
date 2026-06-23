/**
 * @file ui/tests/qml/views/import/tst_TransactionDraftMetadataPanel.qml
 * @brief Provides QML tests for TransactionDraftMetadataPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Import 1.0 as Import

import "../../common"

TestCase {
    id: testCase
    name: "TransactionDraftMetadataPanelTests"
    when: windowShown
    width: 900
    height: 260

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var importViewModel: QtObject {
        property string metadataText: "Kundennr: 12345"
    }

    Component {
        id: panelComponent
        Import.TransactionDraftMetadataPanel {
            width: testCase.width
            theme: testCase.theme
            importViewModel: testCase.importViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_IMP_D_008A_metadataPanelRendersTransactionStateText() {
        const panel = createTemporaryObject(panelComponent, testCase)

        compare(findRequired(panel, "transactionDraftMetadataTextArea").text, "Kundennr: 12345")
    }
}
