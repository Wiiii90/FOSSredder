/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftForm.qml
 * @brief Provides basic transaction draft fields.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

ColumnLayout {
    id: root
    required property var theme
    required property var transactionViewModel
    spacing: root.theme.spacingSmall

    TransactionDraftFieldRow {
        theme: root.theme
        Layout.fillWidth: true
        leftLabel: qsTr("Name")
        rightLabel: qsTr("Status")
        leftWeight: 3
        rightWeight: 2

        leftContent: Component {
            Controls.TextField {
                objectName: "transactionDraftNameField"
                text: root.transactionViewModel.nameText
                onTextEdited: root.transactionViewModel.nameText = text
                onEditingFinished: root.transactionViewModel.commitNameText()
                onAccepted: root.transactionViewModel.commitNameText()
                onActiveFocusChanged: if (!activeFocus)
                    root.transactionViewModel.commitNameText()
            }
        }

        rightContent: Component {
            Controls.DropdownMenu {
                objectName: "transactionDraftStatusCombo"
                textRole: "label"
                model: root.transactionViewModel.statusOptions
                currentIndex: root.transactionViewModel.statusIndex
                onActivated: function (index) {
                    root.transactionViewModel.selectStatusAtIndex(index);
                }
            }
        }
    }

    TransactionDraftFieldRow {
        theme: root.theme
        Layout.fillWidth: true
        leftLabel: qsTr("Booking Date")
        rightLabel: qsTr("Valuta")
        leftWeight: 3
        rightWeight: 2

        leftContent: Component {
            Controls.TextField {
                objectName: "transactionDraftBookingDateField"
                text: root.transactionViewModel.bookingDateText
                onTextEdited: root.transactionViewModel.bookingDateText = text
                onEditingFinished: root.transactionViewModel.commitBookingDateText()
                onAccepted: root.transactionViewModel.commitBookingDateText()
                onActiveFocusChanged: if (!activeFocus)
                    root.transactionViewModel.commitBookingDateText()
            }
        }

        rightContent: Component {
            Controls.TextField {
                objectName: "transactionDraftValutaField"
                text: root.transactionViewModel.valutaText
                onTextEdited: root.transactionViewModel.valutaText = text
                onEditingFinished: root.transactionViewModel.commitValutaText()
                onAccepted: root.transactionViewModel.commitValutaText()
                onActiveFocusChanged: if (!activeFocus)
                    root.transactionViewModel.commitValutaText()
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
            objectName: "transactionDraftAmountField"
            Layout.fillWidth: true
            text: root.transactionViewModel.amountText
            onTextEdited: root.transactionViewModel.amountText = text
            onEditingFinished: root.transactionViewModel.commitAmountText()
            onAccepted: root.transactionViewModel.commitAmountText()
            onActiveFocusChanged: if (!activeFocus)
                root.transactionViewModel.commitAmountText()
        }
    }
}
