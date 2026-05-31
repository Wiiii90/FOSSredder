/**
 * @file ui/qml/FossRedder/Controls/SuccessButton.qml
 * @brief Provides the SuccessButton component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import FossRedder 1.0

Button {
    fillColor: Theme.buttonConfirmFill
    textColor: Theme.buttonConfirmText
    borderColor: Theme.buttonConfirmBorder
    hoverBorderColor: Theme.accent
    bordered: true
    filled: true
}
