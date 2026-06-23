/**
 * @file ui/qml/FossRedder/Controls/Button.qml
 * @brief Provides the Button component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0

Button {
    id: control
    hoverEnabled: true

    property color fillColor: Theme.buttonFill
    property color textColor: Theme.buttonText
    property color borderColor: Theme.border
    property color hoverBorderColor: Theme.accent
    property bool bordered: false
    property bool filled: false
    property bool emphasized: true

    font.family: Theme.fontFamily
    font.pointSize: Theme.buttonFontSize
    implicitWidth: contentItem.implicitWidth + 32
    implicitHeight: Math.max(Theme.buttonMinHeight, contentItem.implicitHeight + 12)
    focusPolicy: Qt.NoFocus

    function clearVisualState() {
        control.down = false;
    }

    background: Item {
        anchors.fill: parent

        Rectangle {
            anchors.fill: parent
            y: control.pressed ? 1 : (control.hovered ? 2 : 4)
            radius: Theme.radius
            color: Theme.shadow
            opacity: control.enabled ? (control.pressed ? 0.12 : (control.hovered ? 0.09 : 0.04)) : 0.03
            z: -2
            Behavior on y {
                NumberAnimation {
                    duration: 220
                    easing.type: Easing.OutQuad
                }
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: 220
                    easing.type: Easing.OutQuad
                }
            }
        }

        Rectangle {
            id: bg
            anchors.fill: parent
            radius: Theme.radius
            color: control.fillColor
            border.width: control.hovered ? 0.9 : ((control.bordered || !control.filled) ? 1.0 : 0)
            border.color: control.hovered ? control.hoverBorderColor : control.borderColor
            scale: control.pressed ? 0.985 : (control.hovered ? 1.02 : 1.0)
            z: 0

            Behavior on scale {
                NumberAnimation {
                    duration: 220
                    easing.type: Easing.OutQuad
                }
            }
            Behavior on color {
                ColorAnimation {
                    duration: 180
                    easing.type: Easing.OutQuad
                }
            }
        }

        Rectangle {
            anchors.fill: parent
            radius: Theme.radius
            z: 1
            opacity: control.enabled ? (control.hovered ? 0.22 : 0.0) : 0.0
            color: Theme.subtlePrimaryFill
            Behavior on opacity {
                NumberAnimation {
                    duration: 260
                    easing.type: Easing.OutQuad
                }
            }
        }

        Rectangle {
            anchors.fill: parent
            radius: Theme.radius
            color: Theme.shadow
            opacity: control.enabled ? (control.pressed ? 0.06 : 0.0) : 0.02
            z: 2
            Behavior on opacity {
                NumberAnimation {
                    duration: 140
                    easing.type: Easing.OutQuad
                }
            }
        }

        Rectangle {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            height: 1
            color: Theme.surface
            opacity: control.enabled ? 0.012 : 0.0
            z: 3
        }
    }

    contentItem: Text {
        text: control.text
        anchors.centerIn: parent
        color: control.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.family: Theme.fontFamily
        font.pointSize: Theme.buttonFontSize
        font.weight: control.emphasized ? Font.Medium : Font.Normal
        elide: Text.ElideRight
    }

    states: State {
        name: "disabled"
        when: !control.enabled
        PropertyChanges {
            control.contentItem.opacity: 0.55
        }
        PropertyChanges {
            bg.opacity: 0.7
        }
    }

    transitions: Transition {
        NumberAnimation {
            properties: "opacity, scale"
            duration: 220
            easing.type: Easing.OutQuad
        }
    }

    onClicked: control.clearVisualState()
    onPressedChanged: {
        if (!pressed)
            control.clearVisualState();
    }

    Accessible.name: control.text
}
