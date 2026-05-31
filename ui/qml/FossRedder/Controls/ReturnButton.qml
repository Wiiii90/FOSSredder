/**
 * @file ui/qml/FossRedder/Controls/ReturnButton.qml
 * @brief Provides the shared return-to-overview navigation button.
 */

import QtQuick 2.15
import FossRedder 1.0
pragma ComponentBehavior: Bound

FixedSizeButton {
    text: "\u21A9"
    bordered: true
    buttonWidth: Theme.viewNavigationButtonWidth
    buttonHeight: Theme.buttonMinHeight

    Accessible.name: qsTr("Return")
}
