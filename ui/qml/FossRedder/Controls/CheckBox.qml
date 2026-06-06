/**
 * @file ui/qml/FossRedder/Controls/CheckBox.qml
 * @brief Provides the CheckBox component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

CheckBox {
    id: control
    Layout.fillWidth: true
    Layout.alignment: Qt.AlignVCenter
    hoverEnabled: true

    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize
    palette.text: Theme.textPrimary
    palette.windowText: Theme.textPrimary
    palette.buttonText: Theme.textPrimary
    palette.highlight: Theme.primary
    palette.highlightedText: Theme.onPrimary

    spacing: Theme.spacingSmall
    implicitHeight: Math.max(Theme.checkboxSize, contentItem.implicitHeight)
    rightPadding: Theme.spacingSmall

    indicator: Rectangle {
        implicitWidth: Theme.checkboxSize
        implicitHeight: Theme.checkboxSize
        x: control.leftPadding
        y: (control.height - height) / 2
        radius: Theme.radius
        color: control.checked ? Theme.checkboxCheckedFill : (control.hovered ? Theme.checkboxHoverFill : Theme.checkboxFill)
        border.width: Theme.borderWidthThin
        border.color: control.hovered ? Theme.selectionBorder : Theme.checkboxBorder

        Text {
            anchors.centerIn: parent
            text: "✓"
            visible: control.checked
            color: Theme.checkboxCheck
            font.family: Theme.fontFamily
            font.pointSize: Theme.fontSizeSmall
            font.bold: true
        }

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

    contentItem: Text {
        leftPadding: control.indicator.width + control.spacing
        rightPadding: control.rightPadding
        text: control.text
        font.family: Theme.fontFamily
        font.pointSize: Theme.fontSize
        color: control.enabled ? Theme.textPrimary : Theme.textMuted
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
