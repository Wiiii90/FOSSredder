/**
 * @file ui/tests/qml/controls/tst_DropdownMenu.qml
 * @brief Provides QML tests for DropdownMenu behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Controls 1.0

TestCase {
    id: testCase
    name: "DropdownMenuTests"
    when: windowShown
    width: 480
    height: 320

    Component {
        id: dropdownComponent
        DropdownMenu {
            model: ["A", "B", "C"]
            currentIndex: 0
        }
    }

    function createControl() {
        return createTemporaryObject(dropdownComponent, testCase)
    }

    function test_CTRL_DD_001_modelAndCurrentIndexAreApplied() {
        var control = createControl()

        compare(control.model.length, 3)
        compare(control.currentIndex, 0)
    }

    function test_CTRL_DD_002_currentIndexCanChange() {
        var control = createControl()
        control.currentIndex = 2

        compare(control.currentIndex, 2)
    }

}
