/**
 * @file ui/qml/FossRedder/Components/Sidebar.qml
 * @brief Provides the Sidebar component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
pragma ComponentBehavior: Bound

Item {
    id: sidebar
    clip: true

    property Component contentComponent: null

    Rectangle {
        anchors.fill: parent
        color: Theme.surfaceAlt
        border.width: Theme.borderWidthThin
        border.color: Theme.borderLight
    }

    ColumnLayout {
        id: col
        anchors.fill: parent
        anchors.leftMargin: Theme.spacing
        anchors.rightMargin: Theme.spacing
        anchors.bottomMargin: Theme.spacing
        anchors.topMargin: headerLabel.text && headerLabel.text.length > 0 ? Theme.spacing : 0
        spacing: Theme.spacing

        Label {
            id: headerLabel
            text: ""
            font.pointSize: Theme.fontSizeTitle
            color: Theme.textPrimary
            visible: headerLabel.text && headerLabel.text.length > 0
        }

        Rectangle {
            id: headerSep
            Layout.preferredHeight: Theme.borderWidthThin
            color: Theme.borderLight
            Layout.fillWidth: true
            visible: headerLabel.visible
        }

        Loader {
            id: content
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: sidebar.contentComponent
        }
    }
}
