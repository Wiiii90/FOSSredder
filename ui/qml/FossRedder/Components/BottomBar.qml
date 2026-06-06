/**
 * @file ui/qml/FossRedder/Components/BottomBar.qml
 * @brief Provides the BottomBar component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    default property alias content: contentRow.data

    property real horizontalPadding: root.theme.spacing
    property real verticalPadding: root.theme.spacing

    implicitWidth: contentRow.implicitWidth + horizontalPadding * 2
    implicitHeight: contentRow.implicitHeight + verticalPadding * 2

    Rectangle {
        anchors.fill: parent
        radius: root.theme.radius
        antialiasing: true
        color: root.theme.surface
        border.width: root.theme.borderWidthThin
        border.color: root.theme.border

        RowLayout {
            id: contentRow
            Layout.fillWidth: true
            Layout.fillHeight: true
            anchors.fill: parent
            anchors.leftMargin: root.horizontalPadding
            anchors.rightMargin: root.horizontalPadding
            anchors.topMargin: root.verticalPadding
            anchors.bottomMargin: root.verticalPadding
            spacing: root.theme.spacingSmall
        }
    }
}
