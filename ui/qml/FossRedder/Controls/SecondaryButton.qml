/**
 * @file ui/qml/FossRedder/Controls/SecondaryButton.qml
 * @brief Provides the SecondaryButton component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import FossRedder 1.0

Button {
    fillColor: Theme.buttonFill
    textColor: Theme.buttonText
    bordered: true
    filled: false
}
