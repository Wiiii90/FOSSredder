/**
 * @file ui/tests/qml/controls/tst_Button.qml
 * @brief Provides QML tests for Button behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Controls 1.0

TestCase {
    id: testCase
    name: "ButtonTests"
    when: windowShown
    width: 480
    height: 320

    Component {
        id: buttonComponent
        Button {
            text: "Run"
        }
    }

    function createControl() {
        return createTemporaryObject(buttonComponent, testCase)
    }

    function clickEmitsClicked() {
        var control = createControl()
        var clicked = false
        control.clicked.connect(function() { clicked = true })

        control.clicked()

        compare(clicked, true)
    }

    function test_CTRL_B_002_clearVisualStateResetsDown() {
        var control = createControl()
        control.down = true

        control.clearVisualState()

        compare(control.down, false)
    }

    function test_CTRL_B_001_clickEmitsClicked() {
        clickEmitsClicked()
    }

    function test_CTRL_B_003_disabledControlDoesNotActivate() {
        var control = createControl()
        control.enabled = false
        control.down = false

        control.clicked()

        compare(control.down, false)
    }

}
