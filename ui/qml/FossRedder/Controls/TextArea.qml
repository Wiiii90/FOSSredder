/**
 * @file ui/qml/FossRedder/Controls/TextArea.qml
 * @brief Provides the TextArea component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
pragma ComponentBehavior: Bound

TextArea {
    id: control
    Layout.fillWidth: true
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize
    wrapMode: TextArea.Wrap

    background: Rectangle {
        color: Theme.surface
        radius: Theme.radius
        border.color: Theme.borderMedium
        border.width: Theme.borderWidthThin
    }
}
