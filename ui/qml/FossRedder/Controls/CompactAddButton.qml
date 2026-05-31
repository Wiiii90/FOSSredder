/**
 * @file ui/qml/FossRedder/Controls/CompactAddButton.qml
 * @brief Provides a shared compact add action button.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import FossRedder 1.0

FixedSizeButton {
    text: qsTr("+")
    textColor: Theme.buttonMutedText
    buttonWidth: Theme.viewCompactActionButtonSize
    buttonHeight: Theme.viewCompactActionButtonSize

    Accessible.name: qsTr("Add")
}
