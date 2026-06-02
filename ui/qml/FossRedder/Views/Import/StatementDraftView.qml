/**
 * @file ui/qml/FossRedder/Views/Import/StatementDraftView.qml
 * @brief Composes statement draft review.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0 as App
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Import 1.0 as Import

Item {
    id: root
    required property var theme
    required property var importViewModel

    App.StatementDraftViewModel {
        id: statementViewModel
        objectName: "statementDraftViewModel"
        importWorkflow: root.importViewModel.importWorkflow
        workspace: root.importViewModel.workspace
    }

    App.TransactionDraftViewModel {
        id: transactionViewModel
        objectName: "transactionDraftViewModel"
        importWorkflow: root.importViewModel.importWorkflow
        workspace: root.importViewModel.workspace
    }

    ColumnLayout {
        id: layout
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Label {
            color: root.theme.textPrimary
            objectName: "statementDraftEmptyLabel"
            visible: !statementViewModel.hasDraft
            text: qsTr("No drafts available!")
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 14
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }

        Import.StatementDraftForm {
            visible: statementViewModel.hasDraft
            Layout.fillWidth: true
            theme: root.theme
            statementViewModel: statementViewModel
        }

        Controls.Panel {
            visible: statementViewModel.hasDraft
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentSpacing: 0

            Flickable {
                id: txScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 0
                clip: true
                contentWidth: width
                contentHeight: transactionDraftContent.implicitHeight

                ScrollBar.vertical: Controls.AppScrollBar {}

                Import.TransactionDraftView {
                    id: transactionDraftContent
                    width: Math.max(0, txScroll.width - root.theme.scrollBarGutterWidth)
                    theme: root.theme
                    transactionViewModel: transactionViewModel
                }
            }
        }

        Import.StatementDraftBottomBar {
            Layout.fillWidth: true
            theme: root.theme
            importViewModel: root.importViewModel
            statementViewModel: statementViewModel
        }
    }
}
