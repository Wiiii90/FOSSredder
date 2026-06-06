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
    required property var importViewModel
    required property var theme
    property bool embedded: false

    readonly property int suggestionTone: root.importViewModel.suggestionTone(root.importViewModel.actorSuggestionConfidence)
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
                            model: root.importViewModel.actorOptions
                            currentIndex: root.importViewModel.selectedActorOptionIndex
                            onActivated: function(index) { root.importViewModel.selectActorAtIndex(index) }
                        }
                    }

                    rightContent: Component {
                        RowLayout {
                            spacing: root.theme.spacingSmall

                            Controls.TextField {
                                objectName: "transactionDraftActorNameField"
                                Layout.fillWidth: true
                                placeholderText: ""
                                text: root.importViewModel.actorName
                                onTextEdited: root.importViewModel.actorName = text
                                onAccepted: root.importViewModel.actorName = text
                                onEditingFinished: root.importViewModel.actorName = text
                                onActiveFocusChanged: if (!activeFocus) root.importViewModel.actorName = text
                            }

                            Controls.CompactAddButton {
                                objectName: "transactionDraftActorAddFromTextButton"
                                enabled: root.importViewModel.canAddActor
                                onClicked: root.importViewModel.addActor()
                            }
                        }
                    }
                }
            }
        }

        Label {
            objectName: "transactionDraftActorSuggestionLabel"
            text: root.importViewModel.actorSuggestionSummary
            color: root.suggestionColor
            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.theme.spacingSmall
        }
    }
}
