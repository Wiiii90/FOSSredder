/**
 * @file ui/qml/FossRedder/Views/Booking/BookingTransactionActorPanel.qml
 * @brief Provides the BookingTransactionActorPanel component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var bookingState

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    contentSpacing: root.theme.spacingSmall

    background: Rectangle {
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: 1
        border.color: root.theme.border
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: root.theme.spacingSmall

        Label {
            color: root.theme.textPrimary
            text: qsTr("Actor")
            Layout.fillWidth: true
        }

        Controls.DropdownMenu {
            id: actorCombo
            objectName: "bookingTransactionActorComboBox"
            Layout.fillWidth: true
            textRole: "display"
            model: root.bookingState.actorDisplayRows
            currentIndex: root.bookingState.selectedActorIndex
            onActivated: function (index) {
                root.bookingState.selectActorIndex(index);
            }
        }
    }
}
