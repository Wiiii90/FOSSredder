/**
 * @file ui/tests/qml/controls/tst_CheckBox.qml
 * @brief Provides QML tests for CheckBox behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0

TestCase {
    id: testCase
    name: "CheckBoxTests"
    when: windowShown
    width: 480
    height: 320

    Component {
        id: checkBoxComponent
        RowLayout {
            CheckBox {
                id: checkBoxControl
                objectName: "checkBoxControl"
                text: "Option"
                checked: false
            }
        }
    }

    function createControl() {
        return createTemporaryObject(checkBoxComponent, testCase)
    }

    function test_CTRL_CB_001_checkedStateCanBeChanged() {
        var root = createControl()
        var control = root.children[0]
        compare(control.checked, false)

        control.checked = true

        compare(control.checked, true)
    }

    function test_CTRL_CB_002_layoutBindingIsPresent() {
        var root = createControl()
        var control = root.children[0]
        compare(control.Layout.fillWidth, true)
        compare(control.Layout.alignment, Qt.AlignVCenter)
    }

}
