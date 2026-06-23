/**
 * @file ui/qml/FossRedder/Components/ToolbarDivider.qml
 * @brief Provides a vertical divider between toolbar groups.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.3

Rectangle {
    id: root
    required property var theme

    Layout.preferredWidth: root.theme.borderWidthThin
    Layout.fillHeight: true
    Layout.alignment: Qt.AlignVCenter
    color: root.theme.divider
}
