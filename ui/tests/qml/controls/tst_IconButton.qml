/**
 * @file ui/tests/qml/controls/tst_IconButton.qml
 * @brief Provides QML tests for IconButton behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Controls 1.0

TestCase {
    id: testCase
    name: "IconButtonTests"
    when: windowShown
    width: 480
    height: 320

    Component {
        id: iconButtonComponent
        IconButton {
            label: "Import"
            active: false
        }
    }

    function createControl() {
        return createTemporaryObject(iconButtonComponent, testCase)
    }

    function test_CTRL_IB_001_clickSignalIsEmitted() {
        const control = createControl()
        var clicked = false
        control.clicked.connect(function() { clicked = true })

        control.clicked()

        compare(clicked, true)
    }

    function test_CTRL_IB_002_activeStateIsWritable() {
        const control = createControl()

        compare(control.active, false)
        control.active = true
        compare(control.active, true)
    }
}
