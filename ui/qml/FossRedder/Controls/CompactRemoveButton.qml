/**
 * @file ui/qml/FossRedder/Controls/CompactRemoveButton.qml
 * @brief Provides a shared compact remove action button.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import FossRedder 1.0

FixedSizeButton {
    text: qsTr("-")
    textColor: Theme.buttonMutedText
    buttonWidth: Theme.viewCompactActionButtonSize
    buttonHeight: Theme.viewCompactActionButtonSize

    Accessible.name: qsTr("Remove")
}
