/**
 * @file ui/qml/FossRedder/Components/StatusBar.qml
 * @brief Provides the StatusBar component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: bar
    required property var statusState
    required property var theme
    readonly property string readyStatusText: qsTr("Ready")

    Rectangle {
        anchors.fill: parent
        color: bar.theme.statusBarBackground
        border.width: bar.theme.borderWidthThin
        border.color: bar.theme.statusBarBorder

        RowLayout {
            anchors.fill: parent
            anchors.margins: bar.theme.spacingMedium

            Label {
                id: lblStatus
                text: bar.statusState.text.length > 0
                      ? bar.statusState.text
                      : bar.readyStatusText
                color: bar.theme.textPrimary
                elide: Label.ElideRight
                Layout.fillWidth: true
            }
        }
    }
}
