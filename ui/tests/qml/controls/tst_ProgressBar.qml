/**
 * @file ui/tests/qml/controls/tst_ProgressBar.qml
 * @brief Provides QML tests for ProgressBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Controls 1.0

TestCase {
    id: testCase
    name: "ProgressBarTests"
    when: windowShown
    width: 480
    height: 320

    Component {
        id: progressBarComponent
        ProgressBar {
            width: 300
            value: 0.4
        }
    }

    function createControl() {
        return createTemporaryObject(progressBarComponent, testCase)
    }

    function test_CTRL_PB_001_valuePropertyIsApplied() {
        var control = createControl()
        compare(control.value, 0.4)
    }

    function test_CTRL_PB_002_indeterminateCanBeEnabled() {
        var control = createControl()
        control.indeterminate = true

        compare(control.indeterminate, true)
    }

}
