/**
 * @file ui/qml/FossRedder/Controls/PrevPageButton.qml
 * @brief Provides the shared jump-back navigation button.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
pragma ComponentBehavior: Bound

Button {
    text: "⟪"
    bordered: true
    Layout.preferredWidth: Theme.viewNavigationButtonWidth
}
