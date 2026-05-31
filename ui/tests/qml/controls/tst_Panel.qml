/**
 * @file ui/tests/qml/controls/tst_Panel.qml
 * @brief Provides QML tests for Panel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Controls 1.0

TestCase {
    id: testCase
    name: "PanelTests"
    when: windowShown
    width: 480
    height: 320

    Component {
        id: panelComponent
        Panel {
            contentSpacing: 9
        }
    }

    function createControl() {
        return createTemporaryObject(panelComponent, testCase)
    }

    function test_CTRL_P_002_contentSpacingPropertyIsApplied() {
        var control = createControl()

        compare(control.contentSpacing, 9)
    }

    function test_CTRL_P_001_contentContainerIsAvailable() {
        var control = createControl()
        verify(control.contentItem !== null)
    }

}
