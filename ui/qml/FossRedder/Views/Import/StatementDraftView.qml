/**
 * @file ui/qml/FossRedder/Views/Import/StatementDraftView.qml
 * @brief Composes statement draft review.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0 as App
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Import 1.0 as Import
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var importState
    property var draft

    App.StatementDraftState {
        id: statementState
        objectName: "statementDraftState"
        importWorkflow: root.importState.importWorkflow
        navigation: root.importState.navigation
        draft: root.draft
        transactionState: transactionState
    }

    App.TransactionDraftState {
        id: transactionState
        objectName: "transactionDraftState"
        importWorkflow: root.importState.importWorkflow
        workspace: root.importState.workspace
        draft: root.draft
    }

    ColumnLayout {
        id: layout
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Label {
            objectName: "statementDraftEmptyLabel"
            visible: !statementState.hasDraft
            text: qsTr("No drafts available!")
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 14
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }

        Import.StatementDraftForm {
            visible: statementState.hasDraft
            Layout.fillWidth: true
            theme: root.theme
            statementState: statementState
        }

        Controls.Panel {
            visible: statementState.hasDraft
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentSpacing: 0

            ScrollView {
                id: txScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 0
                clip: true

                Import.TransactionDraftView {
                    width: txScroll.availableWidth > 0 ? txScroll.availableWidth : txScroll.width
                    theme: root.theme
                    transactionState: transactionState
                }
            }
        }

        Import.StatementDraftBottomBar {
            Layout.fillWidth: true
            theme: root.theme
            statementState: statementState
        }
    }
}
