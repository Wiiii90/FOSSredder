/**
 * @file ui/qml/FossRedder/Controls/DangerButton.qml
 * @brief Provides the DangerButton component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import FossRedder 1.0

Button {
    fillColor: Theme.buttonFill
    textColor: Theme.buttonText
    borderColor: Theme.borderMedium
    hoverBorderColor: Theme.danger
    bordered: true
    filled: false
    emphasized: false
}
