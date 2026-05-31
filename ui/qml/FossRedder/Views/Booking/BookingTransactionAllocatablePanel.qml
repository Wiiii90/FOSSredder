/**
 * @file ui/qml/FossRedder/Views/Booking/BookingTransactionAllocatablePanel.qml
 * @brief Provides the BookingTransactionAllocatablePanel component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
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

        Controls.Button {
            objectName: "bookingTransactionAllocatableToggle"
            Layout.fillWidth: true
            Layout.preferredHeight: root.theme.controlHeight
            fillColor: root.theme.surface
            textColor: root.theme.textPrimary
            bordered: true
            filled: false
            emphasized: false
            text: root.bookingState.transactionAllocatable ? qsTr("Allocatable") : qsTr("Not allocatable")
            onClicked: root.bookingState.transactionAllocatable = !root.bookingState.transactionAllocatable
        }
    }
}
