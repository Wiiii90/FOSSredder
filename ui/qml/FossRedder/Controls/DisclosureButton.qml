/**
 * @file ui/qml/FossRedder/Controls/DisclosureButton.qml
 * @brief Provides the shared expand/collapse disclosure button.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Button {
    property bool expanded: false

    text: expanded ? "\u25BC" : "\u25B6"
    bordered: true
    fillColor: "transparent"
    textColor: Theme.buttonMutedText
    implicitWidth: Theme.viewNavigationButtonWidth
    implicitHeight: Theme.viewCompactActionButtonSizeSmall
    Layout.preferredWidth: Theme.viewNavigationButtonWidth
    Layout.minimumWidth: Theme.viewNavigationButtonWidth
    Layout.maximumWidth: Theme.viewNavigationButtonWidth
    Layout.preferredHeight: Theme.viewCompactActionButtonSizeSmall
    Layout.minimumHeight: Theme.viewCompactActionButtonSizeSmall
    Layout.maximumHeight: Theme.viewCompactActionButtonSizeSmall

    Accessible.name: expanded ? qsTr("Collapse") : qsTr("Expand")
}
