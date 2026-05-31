/**
 * @file ui/qml/FossRedder/Controls/TextArea.qml
 * @brief Provides the TextArea component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

TextArea {
    id: control
    Layout.fillWidth: true
    hoverEnabled: true
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize
    color: Theme.textPrimary
    placeholderTextColor: Theme.placeholderText
    selectionColor: Theme.primary
    selectedTextColor: Theme.onPrimary
    wrapMode: TextArea.Wrap

    background: Item {
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
}
