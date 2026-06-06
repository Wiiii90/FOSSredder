/**
 * @file ui/tests/qml/controls/tst_CheckListPanel.qml
 * @brief Provides QML tests for CheckListPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtTest 1.3
import FossRedder.Controls 1.0

TestCase {
    id: testCase
    name: "CheckListPanelTests"
    when: windowShown
    width: 480
    height: 320

    Component {
        id: checkListPanelComponent
        CheckListPanel {
            CheckBox {
                text: "A"
            }
            CheckBox {
                text: "B"
            }
        }
    }

    function createControl() {
        return createTemporaryObject(checkListPanelComponent, testCase)
    }

    function test_CTRL_CLP_001_contentHostIsAvailable() {
        const control = createControl()

        verify(control.contentItem !== null)
        verify(control.implicitHeight > 0)
    }

    function test_CTRL_CLP_002_layoutDoesNotFillParent() {
        const control = createControl()

        compare(control.Layout.fillWidth, false)
        compare(control.Layout.fillHeight, false)
    }
}
