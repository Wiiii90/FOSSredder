/**
 * @file ui/qml/FossRedder/Components/AppMenuPopupFrame.qml
 * @brief Provides the themed popup frame used by the application menu.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Effects

Item {
    id: root
    required property var theme

    implicitWidth: Math.max(root.theme.appMenuPopupMinWidth, frame.implicitWidth)
    implicitHeight: frame.implicitHeight

    Rectangle {
        id: shadowSource
        anchors.fill: frame
        radius: frame.radius
        color: root.theme.shadow
        visible: false
    }

    MultiEffect {
        anchors.fill: shadowSource
        source: shadowSource
        shadowEnabled: true
        shadowBlur: 0.34
        shadowColor: root.theme.shadow
        shadowOpacity: root.theme.popupShadowOpacity * 0.55
        shadowVerticalOffset: 3
        shadowHorizontalOffset: 0
    }

    Rectangle {
        id: frame
        anchors.fill: parent
        implicitWidth: root.theme.appMenuPopupMinWidth
        implicitHeight: root.theme.appMenuItemHeight
        radius: root.theme.appMenuPopupRadius
        color: root.theme.appMenuPopupFill
        border.width: root.theme.borderWidthThin
        border.color: root.theme.appMenuPopupBorder
    }
}
