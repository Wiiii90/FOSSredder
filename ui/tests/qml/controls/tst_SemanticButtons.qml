/**
 * @file ui/tests/qml/controls/tst_SemanticButtons.qml
 * @brief Provides QML tests for semantic button variants.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Controls 1.0

TestCase {
    id: testCase
    name: "SemanticButtonTests"
    when: windowShown
    width: 480
    height: 320

    Component { id: addButtonComponent; AddButton {} }
    Component { id: secondaryButtonComponent; SecondaryButton { text: "Secondary" } }
    Component { id: successButtonComponent; SuccessButton { text: "Save" } }
    Component { id: dangerButtonComponent; DangerButton { text: "Delete" } }

    function test_CTRL_SBTN_001_addButtonUsesSharedLabel() {
        const control = createTemporaryObject(addButtonComponent, testCase)

        compare(control.text, "Add")
    }

    function test_CTRL_SBTN_002_secondaryAndDangerAreOutlined() {
        const secondary = createTemporaryObject(secondaryButtonComponent, testCase)
        const danger = createTemporaryObject(dangerButtonComponent, testCase)

        compare(secondary.bordered, true)
        compare(secondary.filled, false)
        compare(danger.bordered, true)
        compare(danger.filled, false)
    }

    function test_CTRL_SBTN_003_successIsFilled() {
        const control = createTemporaryObject(successButtonComponent, testCase)

        compare(control.bordered, true)
        compare(control.filled, true)
    }
}
