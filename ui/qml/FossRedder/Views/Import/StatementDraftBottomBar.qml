/**
 * @file ui/qml/FossRedder/Views/Import/StatementDraftBottomBar.qml
 * @brief Provides statement draft review bottom actions.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var importViewModel
    required property var statementViewModel

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: root
        theme: root.theme

        Controls.PrevPageButton {
            objectName: "statementDraftPrevPageButton"
            enabled: root.importViewModel.hasDraftNavigation
            onClicked: root.importViewModel.selectPreviousDraft()
        }

        Controls.PrevButton {
            objectName: "statementDraftPrevTransactionButton"
            enabled: root.statementViewModel.canSelectPreviousTransactionDraft
            onClicked: root.statementViewModel.selectPreviousTransactionDraft()
        }

        Item { Layout.fillWidth: true }

        Controls.ReturnButton {
            objectName: "statementDraftReturnButton"
            enabled: root.statementViewModel.hasDraft
            onClicked: root.statementViewModel.returnToImport()
        }

        Controls.DangerButton {
            objectName: "statementDraftDiscardButton"
            text: qsTr("Discard")
            enabled: root.statementViewModel.hasDraft
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.statementViewModel.discard()
        }

        Controls.SuccessButton {
            objectName: "statementDraftFinalizeButton"
            text: qsTr("Finalize")
            enabled: root.statementViewModel.hasDraft
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.statementViewModel.finalize()
        }

        Item { Layout.fillWidth: true }

        Controls.NextButton {
            objectName: "statementDraftNextTransactionButton"
            enabled: root.statementViewModel.canSelectNextTransactionDraft
            onClicked: root.statementViewModel.selectNextTransactionDraft()
        }

        Controls.NextPageButton {
            objectName: "statementDraftNextPageButton"
            enabled: root.importViewModel.hasDraftNavigation
            onClicked: root.importViewModel.selectNextDraft()
        }
    }
}
