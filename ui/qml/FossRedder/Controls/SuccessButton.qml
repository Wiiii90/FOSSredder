/**
 * @file ui/qml/FossRedder/Controls/SuccessButton.qml
 * @brief Provides the SuccessButton component.
 */

import QtQuick 2.15
import FossRedder 1.0
pragma ComponentBehavior: Bound

Button {
    fillColor: Qt.lighter(Theme.success, 3.2)
    textColor: Theme.textPrimary
    bordered: true
    filled: false
}
