/**
 * @file ui/tests/qml/controls/tst_TextArea.qml
 * @brief Provides QML tests for TextArea behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Controls 1.0

TestCase {
    id: testCase
    name: "TextAreaTests"
    when: windowShown
    width: 480
    height: 320

    Component {
        id: textAreaComponent
        TextArea {
            text: ""
        }
    }

    function createControl() {
        return createTemporaryObject(textAreaComponent, testCase)
    }

    function test_CTRL_TA_001_textBindingUpdatesValue() {
        const control = createControl()

        control.text = "Long note"

        compare(control.text, "Long note")
    }

    function test_CTRL_TA_002_wrapModeUsesWordWrap() {
        const control = createControl()

        compare(control.wrapMode, TextArea.Wrap)
    }
}
