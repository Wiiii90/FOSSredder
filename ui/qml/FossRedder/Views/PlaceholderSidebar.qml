/**
 * @file ui/qml/FossRedder/Views/PlaceholderSidebar.qml
 * @brief Displays placeholder content for sidebar areas that are not implemented yet.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0

Item {
    anchors.fill: parent

    Label {
        anchors.centerIn: parent
        text: qsTr("Not implemented")
        color: Theme.placeholderText
    }
}
