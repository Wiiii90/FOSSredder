/**
 * @file ui/qml/FossRedder/Controls/YearPicker.qml
 * @brief Provides a compact year selector with text-field-like styling.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0

SpinBox {
    id: control

    from: 1900
    to: 2500
    value: 2026
    editable: false
    implicitHeight: Theme.controlHeight
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize
    palette.text: Theme.textPrimary
    palette.buttonText: Theme.textPrimary
    palette.highlight: Theme.primary
    palette.highlightedText: Theme.onPrimary

    textFromValue: function (v, locale) {
        return String(Math.trunc(v));
    }

    valueFromText: function (t, locale) {
        return parseInt(String(t));
    }
    background: Rectangle {
        color: Theme.surface
        radius: Theme.radius
        border.color: control.activeFocus ? Theme.primary.lighter(140) : Theme.borderMedium
        border.width: Theme.borderWidthThin
        implicitHeight: Theme.controlHeight
        anchors.fill: parent
        Behavior on border.color {
            ColorAnimation {
                duration: 160
            }
        }
    }
}
