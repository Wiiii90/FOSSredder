/**
 * @file ui/qml/FossRedder/Components/ToolbarGroup.qml
 * @brief Provides a labeled group of toolbar icon actions.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.3

ColumnLayout {
    id: root
    required property var theme
    property string label: ""
    property bool active: false
    property int iconRowHeight: root.theme.toolbarIconRowHeight
    readonly property real actionHeight: iconRow.height
    default property alias actions: iconRow.data

    spacing: root.theme.toolbarSectionSpacing
    Layout.alignment: Qt.AlignVCenter
    Layout.preferredHeight: parent ? parent.height : implicitHeight
    Layout.maximumHeight: parent ? parent.height : implicitHeight

    RowLayout {
        id: iconRow
        spacing: root.theme.toolbarGroupSpacing
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.preferredHeight: root.iconRowHeight
    }

    Text {
        text: root.label
        color: root.active ? root.theme.toolbarGroupActiveText : root.theme.textMuted
        font.family: root.theme.fontFamily
        font.pointSize: root.theme.toolbarLabelFontSize
        font.weight: Font.Normal
        horizontalAlignment: Text.AlignHCenter
        Layout.alignment: Qt.AlignHCenter
        elide: Text.ElideRight
        visible: root.label.length > 0
    }

    Item {
        Layout.preferredHeight: root.theme.toolbarContentBottomInset
    }
}
