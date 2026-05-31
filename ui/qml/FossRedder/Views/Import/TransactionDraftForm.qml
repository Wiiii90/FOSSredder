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
    required property var transactionState
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
                text: root.transactionState.nameText
                onTextEdited: root.transactionState.nameText = text
                onEditingFinished: root.transactionState.commitNameText()
                onAccepted: root.transactionState.commitNameText()
                onActiveFocusChanged: if (!activeFocus)
                    root.transactionState.commitNameText()
            }
        }

        rightContent: Component {
            Controls.DropdownMenu {
                objectName: "transactionDraftStatusCombo"
                textRole: "label"
                model: root.transactionState.statusOptions
                currentIndex: root.transactionState.statusIndex
                onActivated: function (index) {
                    root.transactionState.setStatusByIndex(index);
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
                text: root.transactionState.bookingDateText
                onTextEdited: root.transactionState.bookingDateText = text
                onEditingFinished: root.transactionState.commitBookingDateText()
                onAccepted: root.transactionState.commitBookingDateText()
                onActiveFocusChanged: if (!activeFocus)
                    root.transactionState.commitBookingDateText()
            }
        }

        rightContent: Component {
            Controls.TextField {
                objectName: "transactionDraftValutaField"
                text: root.transactionState.valutaText
                onTextEdited: root.transactionState.valutaText = text
                onEditingFinished: root.transactionState.commitValutaText()
                onAccepted: root.transactionState.commitValutaText()
                onActiveFocusChanged: if (!activeFocus)
                    root.transactionState.commitValutaText()
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
            text: root.transactionState.amountText
            onTextEdited: root.transactionState.amountText = text
            onEditingFinished: root.transactionState.commitAmountText()
            onAccepted: root.transactionState.commitAmountText()
            onActiveFocusChanged: if (!activeFocus)
                root.transactionState.commitAmountText()
        }
    }
}
