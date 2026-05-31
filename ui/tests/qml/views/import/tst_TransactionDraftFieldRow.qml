/**
 * @file ui/tests/qml/views/import/tst_TransactionDraftFieldRow.qml
 * @brief Provides QML tests for TransactionDraftFieldRow layout wiring.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "TransactionDraftFieldRowTests"
    when: windowShown
    width: 640
    height: 160

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacingMedium: 8
        property int controlHeight: 32
    }

    Component {
        id: rowComponent
        Import.TransactionDraftFieldRow {
            width: testCase.width
            theme: testCase.theme
            leftLabel: "Left"
            rightLabel: "Right"
            leftContent: Component { Item { objectName: "fieldRowLeftContent"; implicitHeight: 44 } }
            rightContent: Component { Item { objectName: "fieldRowRightContent"; implicitHeight: 36 } }
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_IMP_D_018_fieldRowLoadsBothContentComponents() {
        const row = createTemporaryObject(rowComponent, testCase)

        verify(findRequired(row, "fieldRowLeftContent") !== null)
        verify(findRequired(row, "fieldRowRightContent") !== null)
        verify(row.implicitHeight >= 44)
    }
}
