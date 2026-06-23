/**
 * @file ui/qml/FossRedder/Views/Booking/BookingStatementForm.qml
 * @brief Provides the Booking statement form composition.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Booking 1.0 as Booking

Item {
    id: root

    required property var theme
    required property var bookingViewModel
    property bool readOnly: false

    Layout.fillWidth: true
    Layout.fillHeight: true

    ColumnLayout {
        anchors.fill: root
        spacing: root.theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true

            Label {
                color: root.theme.textPrimary
                text: qsTr("Statement")
                Layout.preferredWidth: root.theme.formLabelWidth
            }

            Controls.TextField {
                objectName: "bookingStatementNameField"
                Layout.fillWidth: true
                readOnly: root.readOnly
                text: root.bookingViewModel.statementName
                onTextEdited: root.bookingViewModel.statementName = text
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                objectName: "bookingTransactionInfoLabel"
                text: root.bookingViewModel.transactionInfoText
                color: root.theme.textMuted
            }

            Item {
                Layout.fillWidth: true
            }

            Controls.CompactAddButton {
                objectName: "bookingStatementAddTransactionButton"
                visible: root.bookingViewModel.canAddTransaction
                enabled: root.bookingViewModel.canAddTransaction
                onClicked: root.bookingViewModel.addTransactionAfterCurrent()
            }

            Controls.CompactRemoveButton {
                objectName: "bookingStatementRemoveTransactionButton"
                visible: true
                enabled: root.bookingViewModel.canDeleteTransaction
                onClicked: root.bookingViewModel.deleteCurrentTransaction()
            }
        }

        Controls.Panel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentSpacing: 0
            background: Rectangle {
                radius: root.theme.radius
                color: root.theme.surface
                border.width: 1
                border.color: root.theme.border
            }

            Flickable {
                id: transactionScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: width
                contentHeight: contentLayout.implicitHeight

                ScrollBar.vertical: Controls.AppScrollBar {}

                ColumnLayout {
                    id: contentLayout
                    width: Math.max(0, transactionScroll.width - root.theme.scrollBarGutterWidth)
                    spacing: root.theme.spacingSmall

                    Booking.BookingTransactionView {
                        Layout.fillWidth: true
                        theme: root.theme
                        bookingViewModel: root.bookingViewModel
                    }
                }
            }
        }
    }
}
