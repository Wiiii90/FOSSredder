/**
 * @file ui/qml/FossRedder/Controls/ProgressBar.qml
 * @brief Provides the ProgressBar component.
 */

import QtQuick 2.15
import FossRedder 1.0
pragma ComponentBehavior: Bound

Item {
    id: root
    implicitHeight: 16

    property real value: 0
    property bool indeterminate: false

    Rectangle {
        anchors.fill: parent
        radius: Theme.radius
        color: Theme.surfaceAlt
        border.width: 1
        border.color: Theme.border
    }

    Rectangle {
        id: fill
        height: parent.height
        radius: Theme.radius
        color: Theme.primary
        width: root.indeterminate ? Math.max(24, parent.width * 0.25) : Math.max(0, Math.min(parent.width, parent.width * root.value))

        SequentialAnimation on x {
            running: root.indeterminate && root.visible
            loops: Animation.Infinite
            NumberAnimation { from: -fill.width; to: root.width; duration: 900; easing.type: Easing.InOutQuad }
        }
    }
}
