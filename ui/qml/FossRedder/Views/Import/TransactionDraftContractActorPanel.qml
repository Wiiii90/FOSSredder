/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftContractActorPanel.qml
 * @brief Edits actor text and actor matching inside the transaction draft contract panel.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var transactionViewModel
    required property var theme
    property bool embedded: false

    readonly property int suggestionTone: root.transactionViewModel.suggestionTone(root.transactionViewModel.actorSuggestionConfidence)
    readonly property color suggestionColor: root.suggestionTone === 2 ? root.theme.successStrong : (root.suggestionTone === 1 ? root.theme.warning : root.theme.danger)

    Layout.fillWidth: true
    Layout.fillHeight: false
    Layout.preferredWidth: 1
    Layout.preferredHeight: implicitHeight
    implicitHeight: contentLayout.implicitHeight

    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Controls.Panel {
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.preferredHeight: implicitHeight
            contentSpacing: root.theme.spacingSmall

            background: Rectangle {
                radius: root.theme.radius
                color: root.theme.surfaceAlt
                border.width: 1
                border.color: root.suggestionColor
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.preferredHeight: implicitHeight
                spacing: root.theme.spacingSmall

                TransactionDraftFieldRow {
                    theme: root.theme
                    Layout.fillWidth: true
                    columnSpacing: root.theme.spacingSmall
                    leftLabel: qsTr("Select Actor")
                    rightLabel: qsTr("Name")
                    leftWeight: 3
                    rightWeight: 2

                    leftContent: Component {
                        Controls.DropdownMenu {
                            objectName: "transactionDraftActorCombo"
                            Layout.fillWidth: true
                            textRole: "display"
                            model: root.transactionViewModel.actorOptions
                            currentIndex: root.transactionViewModel.selectedActorOptionIndex
                            onActivated: function(index) { root.transactionViewModel.selectActorAtIndex(index) }
                        }
                    }

                    rightContent: Component {
                        RowLayout {
                            spacing: root.theme.spacingSmall

                            Controls.TextField {
                                objectName: "transactionDraftActorNameField"
                                Layout.fillWidth: true
                                placeholderText: ""
                                text: root.transactionViewModel.actorName
                                onTextEdited: root.transactionViewModel.actorName = text
                                onAccepted: root.transactionViewModel.actorName = text
                                onEditingFinished: root.transactionViewModel.actorName = text
                                onActiveFocusChanged: if (!activeFocus) root.transactionViewModel.actorName = text
                            }

                            Controls.CompactAddButton {
                                objectName: "transactionDraftActorAddFromTextButton"
                                enabled: root.transactionViewModel.canAddActor
                                onClicked: root.transactionViewModel.addActor()
                            }
                        }
                    }
                }
            }
        }

        Label {
            objectName: "transactionDraftActorSuggestionLabel"
            text: root.transactionViewModel.actorSuggestionSummary
            color: root.suggestionColor
            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.theme.spacingSmall
        }
    }
}
