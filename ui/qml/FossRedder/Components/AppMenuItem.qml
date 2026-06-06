/**
 * @file ui/qml/FossRedder/Components/AppMenuItem.qml
 * @brief Provides a themed menu item for the application menu.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

MenuItem {
    id: root
    required property var theme

    implicitWidth: Math.max(root.theme.appMenuPopupMinWidth, itemContent.implicitWidth + leftPadding + rightPadding)
    implicitHeight: root.theme.appMenuItemHeight
    leftPadding: root.theme.appMenuPopupPadding + root.theme.appMenuItemHorizontalPadding
    rightPadding: root.theme.appMenuPopupPadding + root.theme.appMenuItemHorizontalPadding
    topPadding: 0
    bottomPadding: 0
    font.family: root.theme.fontFamily
    font.pointSize: root.theme.appMenuFontSize

    contentItem: RowLayout {
        id: itemContent
        spacing: root.theme.spacingSmall

        Text {
            text: root.checkable ? (root.checked ? "\u2713" : "") : ""
            color: root.enabled ? root.theme.appMenuPopupText : root.theme.appMenuPopupMutedText
            font.family: root.theme.fontFamily
            font.pointSize: root.theme.appMenuFontSize
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            Layout.preferredWidth: root.theme.appMenuItemCheckWidth
            Layout.fillHeight: true
        }

        Text {
            text: root.text
            color: root.enabled ? root.theme.appMenuPopupText : root.theme.appMenuPopupMutedText
            font.family: root.theme.fontFamily
            font.pointSize: root.theme.appMenuFontSize
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            Layout.fillWidth: true
            Layout.minimumWidth: root.theme.menuItemMinWidth
        }

        Text {
            text: root.action ? String(root.action.shortcut || "") : ""
            color: root.theme.appMenuPopupMutedText
            font.family: root.theme.fontFamily
            font.pointSize: root.theme.appMenuFontSize
            verticalAlignment: Text.AlignVCenter
            visible: text.length > 0
            Layout.leftMargin: root.theme.appMenuItemShortcutSpacing
        }
    }

    background: Rectangle {
        anchors.fill: parent
        anchors.leftMargin: root.theme.appMenuPopupPadding
        anchors.rightMargin: root.theme.appMenuPopupPadding
        radius: root.theme.appMenuItemRadius
        color: root.highlighted ? root.theme.appMenuPopupHoverFill : "transparent"
    }
}
