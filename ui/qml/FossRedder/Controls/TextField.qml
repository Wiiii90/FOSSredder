/**
 * @file ui/qml/FossRedder/Controls/TextField.qml
 * @brief Provides the TextField component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

TextField {
    id: control
    Layout.fillWidth: true
    hoverEnabled: true
    implicitHeight: Theme.controlHeight
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize
    color: Theme.textPrimary
    placeholderTextColor: Theme.placeholderText
    selectionColor: Theme.primary
    selectedTextColor: Theme.onPrimary

    background: Item {
        implicitHeight: Theme.controlHeight
        anchors.fill: parent

        Rectangle {
            anchors.fill: parent
            y: control.activeFocus ? 1 : 2
            radius: Theme.radius
            color: Theme.shadow
            opacity: control.enabled ? Theme.controlShadowOpacity : 0.02
        }

        Rectangle {
            anchors.fill: parent
            radius: Theme.radius
            color: control.hovered || control.activeFocus ? Theme.controlHoverFill : Theme.controlFill
            border.color: control.activeFocus ? Theme.controlFocusBorder : (control.hovered ? Theme.controlHoverBorder : Theme.borderMedium)
            border.width: Theme.borderWidthThin

            Behavior on color {
                ColorAnimation {
                    duration: Theme.animationDurationFast
                }
            }

            Behavior on border.color {
                ColorAnimation {
                    duration: Theme.animationDurationFast
                }
            }
        }
    }

    leftPadding: Theme.controlPaddingHorizontal
    rightPadding: Theme.controlPaddingHorizontal
    topPadding: Theme.controlPaddingVertical
    bottomPadding: Theme.controlPaddingVertical
}
