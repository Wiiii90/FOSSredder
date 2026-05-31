/**
 * @file ui/tests/qml/controls/tst_TextField.qml
 * @brief Provides QML tests for TextField behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Controls 1.0

TestCase {
    id: testCase
    name: "TextFieldTests"
    when: windowShown
    width: 480
    height: 320

    Component {
        id: textFieldComponent
        TextField {
            text: ""
        }
    }

    function createControl() {
        return createTemporaryObject(textFieldComponent, testCase)
    }

    function test_CTRL_TF_001_textBindingUpdatesValue() {
        var control = createControl()
        control.text = "Hello"

        compare(control.text, "Hello")
    }

    function test_CTRL_TF_002_placeholderTextIsWritable() {
        var control = createControl()
        control.placeholderText = "Enter value"

        compare(control.placeholderText, "Enter value")
    }

}
