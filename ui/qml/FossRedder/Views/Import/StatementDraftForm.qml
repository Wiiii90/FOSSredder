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
    required property var statementState
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
            text: root.statementState.statementName
            onTextEdited: root.statementState.statementName = text
            onEditingFinished: root.statementState.statementName = text
            onAccepted: root.statementState.statementName = text
            onActiveFocusChanged: if (!activeFocus)
                root.statementState.statementName = text
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Label {
            color: root.theme.textPrimary
            Layout.fillWidth: true
            text: root.statementState.transactionInfoText
        }

        Controls.CompactAddButton {
            objectName: "statementDraftAddTransactionButton"
            visible: root.statementState.hasDraft
            onClicked: root.statementState.addTransactionAfterCurrent()
        }

        Controls.CompactRemoveButton {
            objectName: "statementDraftDeleteTransactionButton"
            visible: root.statementState.canDeleteTransaction
            onClicked: root.statementState.deleteCurrentTransaction()
        }
    }
}
