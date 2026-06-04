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
    required property var importViewModel
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
                text: root.importViewModel.nameText
                onTextEdited: root.importViewModel.nameText = text
                onEditingFinished: root.importViewModel.commitNameText()
                onAccepted: root.importViewModel.commitNameText()
                onActiveFocusChanged: if (!activeFocus)
                    root.importViewModel.commitNameText()
            }
        }

        rightContent: Component {
            Controls.DropdownMenu {
                objectName: "transactionDraftStatusCombo"
                textRole: "label"
                model: root.importViewModel.statusOptions
                currentIndex: root.importViewModel.statusIndex
                onActivated: function (index) {
                    root.importViewModel.selectStatusAtIndex(index);
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
                text: root.importViewModel.bookingDateText
                onTextEdited: root.importViewModel.bookingDateText = text
                onEditingFinished: root.importViewModel.commitBookingDateText()
                onAccepted: root.importViewModel.commitBookingDateText()
                onActiveFocusChanged: if (!activeFocus)
                    root.importViewModel.commitBookingDateText()
            }
        }

        rightContent: Component {
            Controls.TextField {
                objectName: "transactionDraftValutaField"
                text: root.importViewModel.valutaText
                onTextEdited: root.importViewModel.valutaText = text
                onEditingFinished: root.importViewModel.commitValutaText()
                onAccepted: root.importViewModel.commitValutaText()
                onActiveFocusChanged: if (!activeFocus)
                    root.importViewModel.commitValutaText()
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
            text: root.importViewModel.amountText
            onTextEdited: root.importViewModel.amountText = text
            onEditingFinished: root.importViewModel.commitAmountText()
            onAccepted: root.importViewModel.commitAmountText()
            onActiveFocusChanged: if (!activeFocus)
                root.importViewModel.commitAmountText()
        }
    }
}
