/**
 * @file ui/qml/FossRedder/Views/PlaceholderView.qml
 * @brief Displays placeholder page content for views that are not implemented yet.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0

Item {
    Rectangle {
        anchors.fill: parent
        color: Theme.background
        Label {
            anchors.centerIn: parent
            color: Theme.textPrimary
            text: qsTr("Not implemented")
        }
    }
}
