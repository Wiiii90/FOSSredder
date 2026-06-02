/**
 * @file ui/tests/qml/views/import/tst_TransactionDraftMetadataPanel.qml
 * @brief Provides QML tests for TransactionDraftMetadataPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "TransactionDraftMetadataPanelTests"
    when: windowShown
    width: 900
    height: 260

    property var theme: QtObject {
        property int spacingSmall: 6
        property int radius: 3
        property color border: "#cccccc"
        property color surfaceAlt: "#f5f5f5"
    }

    property var transactionViewModel: QtObject {
        property string metadataText: "Kundennr: 12345"
    }

    Component {
        id: panelComponent
        Import.TransactionDraftMetadataPanel {
            width: testCase.width
            theme: testCase.theme
            transactionViewModel: testCase.transactionViewModel
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
