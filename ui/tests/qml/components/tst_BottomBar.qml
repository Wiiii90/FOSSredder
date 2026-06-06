/**
 * @file ui/tests/qml/components/tst_BottomBar.qml
 * @brief Provides QML tests for BottomBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../common" as Common
import FossRedder.Components 1.0
import FossRedder.Controls 1.0 as Controls

TestCase {
    id: testCase
    name: "BottomBarTests"
    when: windowShown
    width: 480
    height: 320

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: bottomBarComponent
        BottomBar {
            width: 400
            theme: testCase.theme

            Controls.Button {
                objectName: "bottomBarChildButton"
                text: "Action"
            }
        }
    }

    function createControl() {
        return createTemporaryObject(bottomBarComponent, testCase)
    }

    function test_CTRL_BB_001_contentSlotHostsChildren() {
        var control = createControl()
        var found = control.children[0]
        verify(found !== null)
    }

    function test_CTRL_BB_002_themeBindingIsApplied() {
        var control = createControl()
        compare(control.theme.radius, 3)
    }

}
