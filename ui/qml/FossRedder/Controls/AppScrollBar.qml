/**
 * @file ui/qml/FossRedder/Controls/AppScrollBar.qml
 * @brief Provides the application themed ScrollBar component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15 as QtControls
import FossRedder 1.0

QtControls.ScrollBar {
    id: root

    property bool persistent: false
    property bool hidden: false
    property bool leading: false
    readonly property real thumbRightInset: Math.max(Theme.borderWidthThin, Theme.scrollBarThumbRightInset)

    policy: root.hidden ? QtControls.ScrollBar.AlwaysOff : (root.persistent ? QtControls.ScrollBar.AlwaysOn : QtControls.ScrollBar.AsNeeded)
    interactive: !root.hidden
    implicitWidth: Theme.scrollBarGutterWidth
    implicitHeight: Theme.scrollBarGutterWidth
    leftPadding: root.leading ? root.thumbRightInset : Math.max(0, Theme.scrollBarGutterWidth - Theme.scrollBarThumbWidth - root.thumbRightInset)
    rightPadding: root.leading ? Math.max(0, Theme.scrollBarGutterWidth - Theme.scrollBarThumbWidth - root.thumbRightInset) : root.thumbRightInset
    topPadding: root.orientation === Qt.Vertical ? Theme.scrollBarThumbEndInset : root.thumbRightInset
    bottomPadding: root.orientation === Qt.Vertical ? Theme.scrollBarThumbEndInset : root.thumbRightInset
    minimumSize: Theme.scrollBarMinimumSize
    opacity: !root.hidden && (root.persistent || root.active || root.hovered || root.pressed) ? 1.0 : 0.0

    Behavior on opacity {
        NumberAnimation {
            duration: Theme.animationDurationFast
        }
    }

    contentItem: Rectangle {
        implicitWidth: Theme.scrollBarThumbWidth
        implicitHeight: Theme.scrollBarThumbWidth
        radius: width / 2
        color: root.hovered || root.pressed ? Theme.scrollBarThumbHover : Theme.scrollBarThumb
    }

    background: Rectangle {
        implicitWidth: Theme.scrollBarGutterWidth
        implicitHeight: Theme.scrollBarGutterWidth
        color: "transparent"
    }
}
