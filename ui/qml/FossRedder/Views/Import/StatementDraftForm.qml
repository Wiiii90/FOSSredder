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
    required property var statementViewModel
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
            text: root.statementViewModel.statementName
            onTextEdited: root.statementViewModel.statementName = text
            onEditingFinished: root.statementViewModel.statementName = text
            onAccepted: root.statementViewModel.statementName = text
            onActiveFocusChanged: if (!activeFocus)
                root.statementViewModel.statementName = text
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Label {
            color: root.theme.textPrimary
            Layout.fillWidth: true
            text: root.statementViewModel.hasDraft
                  ? qsTr("Transaction %1 / %2").arg(root.statementViewModel.currentTransactionNumber).arg(root.statementViewModel.transactionCount)
                  : qsTr("No current transaction")
        }

        Controls.CompactAddButton {
            objectName: "statementDraftAddTransactionButton"
            visible: root.statementViewModel.hasDraft
            onClicked: root.statementViewModel.addTransactionAfterCurrent()
        }

        Controls.CompactRemoveButton {
            objectName: "statementDraftDeleteTransactionButton"
            visible: root.statementViewModel.canDeleteTransaction
            onClicked: root.statementViewModel.deleteCurrentTransaction()
        }
    }
}
