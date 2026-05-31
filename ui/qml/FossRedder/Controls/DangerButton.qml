/**
 * @file ui/qml/FossRedder/Controls/DangerButton.qml
 * @brief Provides the DangerButton component.
 */

import QtQuick 2.15
import FossRedder 1.0
pragma ComponentBehavior: Bound

Button {
    fillColor: Qt.lighter(Theme.danger, 2.2)
    textColor: Theme.textPrimary
    bordered: true
    filled: false
}
