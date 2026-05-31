/**
 * @file ui/qml/FossRedder/Components/ListRow.qml
 * @brief Provides the ListRow component.
 */

import QtQuick 2.15
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    property string text: ""
    property string subtitle: ""
    property url iconSource: ""
    property bool selectable: true
    property bool selected: false
    property color selectedColor: root.theme.background
    property color backgroundColor: "transparent"
    property color borderColor: root.theme.borderSoft
    property real borderWidth: root.theme.borderWidthThin
    property real radius: root.theme.viewSidebarRowRadius
    property int padding: root.theme.spacingSmall

    signal activated()

    implicitHeight: Math.max(root.theme.viewSidebarRowHeight, contentRow.height + root.padding * 2)

    Rectangle {
        id: bg
        anchors.fill: parent
        color: root.selected ? root.selectedColor : root.backgroundColor
        radius: root.radius
        border.color: root.borderColor
        border.width: root.borderWidth

    }

    Row {
        id: contentRow
        anchors.fill: parent
        anchors.margins: root.padding
        spacing: root.theme.spacingSmall

        Image {
            id: icon
            source: root.iconSource
            visible: root.iconSource && String(root.iconSource).length > 0
            width: visible ? root.theme.viewSectionIconSize : 0
            height: visible ? root.theme.viewSectionIconSize : 0
            fillMode: Image.PreserveAspectFit
        }

        Column {
            width: Math.max(0, root.width - (icon.visible ? icon.width + contentRow.spacing : 0) - root.padding * 2)
            spacing: root.theme.viewSidebarRowSpacing

            Text {
                id: title
                text: root.text
                font.family: root.theme.fontFamily
                font.pointSize: root.theme.fontSize
                font.bold: false
                color: root.theme.textPrimary
                elide: Text.ElideRight
                width: parent.width
            }

            Text {
                id: subtitle
                text: root.subtitle
                font.family: root.theme.fontFamily
                font.pointSize: root.theme.fontSizeSmall
                color: root.theme.textMuted
                elide: Text.ElideRight
                visible: root.subtitle && root.subtitle.length > 0
                width: parent.width
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.selectable
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.activated()
    }
}
