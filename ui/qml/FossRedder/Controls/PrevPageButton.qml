/**
 * @file ui/qml/FossRedder/Controls/PrevPageButton.qml
 * @brief Provides the shared jump-back navigation button.
 */

import QtQuick 2.15
import FossRedder 1.0
pragma ComponentBehavior: Bound

FixedSizeButton {
    text: "⟪"
    bordered: true
    buttonWidth: Theme.viewNavigationButtonWidth
    buttonHeight: Theme.buttonMinHeight
}
