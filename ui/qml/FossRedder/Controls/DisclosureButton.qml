/**
 * @file ui/qml/FossRedder/Controls/DisclosureButton.qml
 * @brief Provides the shared expand/collapse disclosure button.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import FossRedder 1.0

FixedSizeButton {
    property bool expanded: false

    text: expanded ? "\u25BC" : "\u25B6"
    bordered: true
    fillColor: "transparent"
    textColor: Theme.buttonMutedText
    buttonWidth: Theme.viewNavigationButtonWidth
    buttonHeight: Theme.viewCompactActionButtonSizeSmall

    Accessible.name: expanded ? qsTr("Collapse") : qsTr("Expand")
}
