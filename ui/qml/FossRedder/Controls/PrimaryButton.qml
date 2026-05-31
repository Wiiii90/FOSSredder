/**
 * @file ui/qml/FossRedder/Controls/PrimaryButton.qml
 * @brief Provides the PrimaryButton component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import FossRedder 1.0

Button {
    fillColor: Theme.primary
    textColor: Theme.onPrimary
    filled: true
}
