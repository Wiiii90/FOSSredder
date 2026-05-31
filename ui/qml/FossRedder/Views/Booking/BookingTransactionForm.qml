/**
 * @file ui/qml/FossRedder/Views/Booking/BookingTransactionForm.qml
 * @brief Provides the booking transaction field form.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var theme
    required property var bookingState

    implicitWidth: formLayout.implicitWidth
    implicitHeight: formLayout.implicitHeight

    ColumnLayout {
        id: formLayout
        width: root.width
        spacing: root.theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true
            spacing: root.theme.spacingMedium

            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 3
                spacing: root.theme.spacingSmall

                Label {
                    color: root.theme.textPrimary
                    text: qsTr("Name")
                    Layout.fillWidth: true
                }

                Controls.TextField {
                    objectName: "bookingTransactionNameField"
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.controlHeight
                    text: root.bookingState.transactionName
                    onTextEdited: root.bookingState.transactionName = text
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 2
                spacing: root.theme.spacingSmall

                Label {
                    color: root.theme.textPrimary
                    text: qsTr("Status")
                    Layout.fillWidth: true
                }

                Controls.DropdownMenu {
                    objectName: "bookingTransactionStatusComboBox"
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.controlHeight
                    textRole: "label"
                    model: root.bookingState.transactionStatusOptions
                    currentIndex: root.bookingState.transactionStatusIndex
                    onActivated: function (index) {
                        root.bookingState.transactionStatusIndex = index;
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: root.theme.spacingMedium

            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 3
                spacing: root.theme.spacingSmall

                Label {
                    color: root.theme.textPrimary
                    text: qsTr("Booking Date")
                    Layout.fillWidth: true
                }

                Controls.TextField {
                    objectName: "bookingTransactionBookingDateField"
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.controlHeight
                    text: root.bookingState.transactionBookingDate
                    onTextEdited: root.bookingState.transactionBookingDate = text
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 2
                spacing: root.theme.spacingSmall

                Label {
                    color: root.theme.textPrimary
                    text: qsTr("Valuta")
                    Layout.fillWidth: true
                }

                Controls.TextField {
                    objectName: "bookingTransactionValutaField"
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.controlHeight
                    text: root.bookingState.transactionValuta
                    onTextEdited: root.bookingState.transactionValuta = text
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: root.theme.spacingSmall

            Label {
                color: root.theme.textPrimary
                text: qsTr("Amount")
                Layout.fillWidth: true
            }

            Controls.TextField {
                objectName: "bookingTransactionAmountField"
                Layout.fillWidth: true
                Layout.preferredHeight: root.theme.controlHeight
                text: root.bookingState.transactionAmountText
                onTextEdited: root.bookingState.transactionAmountText = text
            }
        }
    }
}
