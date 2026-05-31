/**
 * @file ui/qml/FossRedder/Views/Booking/BookingTransactionContractPanel.qml
 * @brief Provides the BookingTransactionContractPanel component.
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
            text: qsTr("Contract")
            Layout.fillWidth: true
        }

        Controls.DropdownMenu {
            id: contractCombo
            objectName: "bookingTransactionContractComboBox"
            Layout.fillWidth: true
            textRole: "display"
            model: root.bookingState.contractDisplayRows
            currentIndex: root.bookingState.selectedContractIndex
            onActivated: function (index) {
                root.bookingState.selectContractIndex(index);
            }
        }
    }
}
