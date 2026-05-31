/**
 * @file ui/qml/FossRedder/Controls/TextField.qml
 * @brief Provides the TextField component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
pragma ComponentBehavior: Bound

TextField {
    id: control
    Layout.fillWidth: true
    implicitHeight: Theme.controlHeight
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize

    background: Rectangle {
        color: Theme.surface
        radius: Theme.radius
        border.color: control.activeFocus ? Theme.primary.lighter(140) : Theme.borderMedium
        border.width: Theme.borderWidthThin
        implicitHeight: Theme.controlHeight
        anchors.fill: parent
        Behavior on border.color { ColorAnimation { duration: 160 } }
    }

    leftPadding: Theme.controlPaddingHorizontal
    rightPadding: Theme.controlPaddingHorizontal
    topPadding: Theme.controlPaddingVertical
    bottomPadding: Theme.controlPaddingVertical
}
