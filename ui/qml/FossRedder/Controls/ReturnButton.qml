/**
 * @file ui/qml/FossRedder/Controls/ReturnButton.qml
 * @brief Provides the shared return-to-overview navigation button.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
pragma ComponentBehavior: Bound

Button {
    text: "\u21A9"
    bordered: true
    Layout.preferredWidth: Theme.viewNavigationButtonWidth

    Accessible.name: qsTr("Return")
}
