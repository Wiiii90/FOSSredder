/**
 * @file ui/qml/FossRedder/Components/AppMenuSeparator.qml
 * @brief Provides a themed separator for the application menu.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15

MenuSeparator {
    id: root
    required property var theme

    leftPadding: 0
    rightPadding: 0
    topPadding: root.theme.spacingSmall
    bottomPadding: root.theme.spacingSmall

    contentItem: Rectangle {
        implicitHeight: root.theme.borderWidthThin
        color: root.theme.appMenuSeparator
    }
}
