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
    required property var transactionState
    required property var theme
    property bool embedded: false

    readonly property int suggestionTone: root.transactionState.suggestionTone(root.transactionState.actorSuggestionConfidence)
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
                            objectName: "transactionDraftActorChoiceCombo"
                            Layout.fillWidth: true
                            textRole: "display"
                            model: root.transactionState.actorChoiceModel
                            currentIndex: root.transactionState.selectedActorIndex
                            onActivated: function(index) { root.transactionState.selectActorIndex(index) }
                        }
                    }

                    rightContent: Component {
                        RowLayout {
                            spacing: root.theme.spacingSmall

                            Controls.TextField {
                                objectName: "transactionDraftActorTextField"
                                Layout.fillWidth: true
                                placeholderText: ""
                                text: root.transactionState.actorText
                                onTextEdited: root.transactionState.actorText = text
                                onAccepted: root.transactionState.actorText = text
                                onEditingFinished: root.transactionState.actorText = text
                                onActiveFocusChanged: if (!activeFocus) root.transactionState.actorText = text
                            }

                            Controls.CompactAddButton {
                                objectName: "transactionDraftActorAddFromTextButton"
                                enabled: root.transactionState.canAddActor
                                onClicked: root.transactionState.addActorFromText()
                            }
                        }
                    }
                }
            }
        }

        Label {
            objectName: "transactionDraftActorSuggestionLabel"
            text: root.transactionState.actorSuggestionSummary
            color: root.suggestionColor
            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.theme.spacingSmall
        }
    }
}
