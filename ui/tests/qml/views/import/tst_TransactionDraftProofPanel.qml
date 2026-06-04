/**
 * @file ui/tests/qml/views/import/tst_TransactionDraftProofPanel.qml
 * @brief Provides QML tests for TransactionDraftProofPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "TransactionDraftProofPanelTests"
    when: windowShown
    width: 900
    height: 260

    readonly property string proofData: "/9j/4AAQSkZJRgABAQAAZABkAAD/2wCEABQQEBkSGScXFycyJh8mMi4mJiYmLj41NTU1NT5EQUFBQUFBREREREREREREREREREREREREREREREREREREREQBFRkZIBwgJhgYJjYmICY2RDYrKzZERERCNUJERERERERERERERERERERERERERERERERERERERERERERERERERP/AABEIAAEAAQMBIgACEQEDEQH/xABMAAEBAAAAAAAAAAAAAAAAAAAABQEBAQAAAAAAAAAAAAAAAAAABQYQAQAAAAAAAAAAAAAAAAAAAAARAQAAAAAAAAAAAAAAAAAAAAD/2gAMAwEAAhEDEQA/AJQA9Yv/2Q=="

    property var theme: QtObject {
        property int spacingSmall: 6
        property int radius: 3
        property color border: "#cccccc"
        property color surfaceAlt: "#f5f5f5"
    }

    property var importViewModel: QtObject {
        property string proofSource: "data:image/jpeg;base64," + testCase.proofData
    }

    Component {
        id: panelComponent
        Import.TransactionDraftProofPanel {
            width: testCase.width
            theme: testCase.theme
            importViewModel: testCase.importViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_IMP_D_008B_proofPanelRendersTransactionStateSource() {
        const panel = createTemporaryObject(panelComponent, testCase)

        compare(findRequired(panel, "transactionDraftProofImage").source.toString(), importViewModel.proofSource)
    }
}
