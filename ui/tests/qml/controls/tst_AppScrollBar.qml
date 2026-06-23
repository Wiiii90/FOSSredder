/**
 * @file ui/tests/qml/controls/tst_AppScrollBar.qml
 * @brief Provides QML tests for AppScrollBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15 as QtControls
import QtTest 1.3
import FossRedder.Controls 1.0

TestCase {
    id: testCase
    name: "AppScrollBarTests"
    when: windowShown
    width: 480
    height: 320

    Component {
        id: scrollBarComponent
        AppScrollBar {}
    }

    function createControl() {
        return createTemporaryObject(scrollBarComponent, testCase)
    }

    function test_CTRL_SB_001_persistentControlsPolicy() {
        const control = createControl()

        compare(control.policy, QtControls.ScrollBar.AsNeeded)
        control.persistent = true
        compare(control.policy, QtControls.ScrollBar.AlwaysOn)
    }

    function test_CTRL_SB_002_hiddenDisablesInteraction() {
        const control = createControl()

        control.hidden = true

        compare(control.policy, QtControls.ScrollBar.AlwaysOff)
        compare(control.interactive, false)
    }
}
