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
    implicitWidth: Theme.viewNavigationButtonWidth
    implicitHeight: Theme.buttonMinHeight
    Layout.preferredWidth: Theme.viewNavigationButtonWidth
    Layout.minimumWidth: Theme.viewNavigationButtonWidth
    Layout.maximumWidth: Theme.viewNavigationButtonWidth
    Layout.preferredHeight: Theme.buttonMinHeight
    Layout.minimumHeight: Theme.buttonMinHeight
    Layout.maximumHeight: Theme.buttonMinHeight

    Accessible.name: qsTr("Return")
}
