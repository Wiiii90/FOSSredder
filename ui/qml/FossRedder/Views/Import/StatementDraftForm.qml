/**
 * @file ui/qml/FossRedder/Views/Import/StatementDraftForm.qml
 * @brief Provides statement draft name and transaction navigation controls.
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

    RowLayout {
        Layout.fillWidth: true

        Label {
            color: root.theme.textPrimary
            text: qsTr("Statement")
            Layout.preferredWidth: root.theme.formLabelWidth
        }

        Controls.TextField {
            objectName: "statementDraftNameField"
            Layout.fillWidth: true
            text: root.importViewModel.statementName
            onTextEdited: root.importViewModel.statementName = text
            onEditingFinished: root.importViewModel.statementName = text
            onAccepted: root.importViewModel.statementName = text
            onActiveFocusChanged: if (!activeFocus)
                root.importViewModel.statementName = text
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Label {
            color: root.theme.textPrimary
            Layout.fillWidth: true
            text: root.importViewModel.hasDraft
                  ? qsTr("Transaction %1 / %2").arg(root.importViewModel.currentTransactionNumber).arg(root.importViewModel.transactionCount)
                  : qsTr("No current transaction")
        }

        Controls.CompactAddButton {
            objectName: "statementDraftAddTransactionButton"
            visible: root.importViewModel.hasDraft
            onClicked: root.importViewModel.addTransactionAfterCurrent()
        }

        Controls.CompactRemoveButton {
            objectName: "statementDraftDeleteTransactionButton"
            visible: root.importViewModel.canDeleteTransaction
            onClicked: root.importViewModel.deleteCurrentTransaction()
        }
    }
}
