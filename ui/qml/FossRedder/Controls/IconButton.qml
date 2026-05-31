/**
 * @file ui/qml/FossRedder/Controls/IconButton.qml
 * @brief Provides the IconButton component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Effects
import FossRedder 1.0

Item {
    id: button
    implicitWidth: Theme.toolbarIconButtonWidth
    implicitHeight: Theme.controlHeight

    width: button.parent && button.parent.width > 0 ? button.parent.width : implicitWidth
    height: button.parent && button.parent.height > 0 ? button.parent.height : implicitHeight

    property url svgSource: ""
    property string label: ""
    property bool active: false

    signal clicked

    Rectangle {
        id: buttonBackground
        anchors.fill: parent
        anchors.topMargin: Theme.toolbarItemFrameTopInset
        radius: Theme.radius
        color: button.active ? Theme.toolbarItemSelectedFill : (buttonMouse.containsMouse ? Theme.toolbarItemHoverFill : "transparent")
        border.width: button.active || buttonMouse.containsMouse ? Theme.borderWidthThin : 0
        border.color: button.active ? Theme.toolbarItemSelectedBorder : Theme.toolbarBorder
    }

    ColumnLayout {
        anchors.fill: buttonBackground
        spacing: Theme.margins
        Layout.alignment: Qt.AlignVCenter

        Item {
            Layout.preferredWidth: Theme.toolbarIconSize
            Layout.preferredHeight: Theme.toolbarIconSize
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            Item {
                id: iconContainer
                anchors.centerIn: parent
                width: Theme.toolbarIconSize
                height: Theme.toolbarIconSize
                property bool hovered: false
                property bool pressed: false
                scale: (hovered ? 1.02 : 1.0) * (pressed ? 0.97 : 1.0)

                Behavior on scale {
                    NumberAnimation {
                        duration: Theme.animationDurationFast
                        easing.type: Easing.OutQuad
                    }
                }

                MultiEffect {
                    anchors.fill: parent
                    source: iconSvg
                    colorization: button.active ? 0.20 : 0
                    colorizationColor: Theme.accent
                    visible: button.active
                    z: 2
                }

                Image {
                    id: iconSvg
                    anchors.centerIn: parent
                    source: button.svgSource
                    width: parent.width
                    height: parent.height
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    opacity: String(button.svgSource).length === 0 ? 0 : (button.active ? 0.96 : 1)
                    visible: status !== Image.Error
                    z: 1
                }
            }
        }

        Text {
            text: button.label
            horizontalAlignment: Text.AlignHCenter
            font.family: Theme.fontFamily
            font.pointSize: Theme.toolbarLabelFontSize
            font.weight: button.active ? Font.Medium : Font.Normal
            color: button.active ? Theme.toolbarItemActiveText : Theme.toolbarItemText
            Layout.alignment: Qt.AlignHCenter
            visible: button.label !== ""
        }
    }

    MouseArea {
        id: buttonMouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        z: 2
        onPressed: iconContainer.pressed = true
        onReleased: iconContainer.pressed = false
        onEntered: iconContainer.hovered = true
        onExited: iconContainer.hovered = false
        onClicked: button.clicked()
    }
}
