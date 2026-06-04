/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftContractPanel.qml
 * @brief Selects and updates contract assignment for the current transaction draft.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Import 1.0 as Import

Item {
    id: root
    required property var importViewModel
    required property var theme

    readonly property real splitColumnSpacing: root.theme.panelPadding + (root.theme.borderWidthThin * 2)
    readonly property real contractMainLeftWeight: 3.2
    readonly property real contractMainRightWeight: 1.8
    readonly property real contractInnerNameWeight: 2
    readonly property real contractInnerTypeWeight: 1.2
    readonly property int suggestionTone: root.importViewModel.suggestionTone(root.importViewModel.contractSuggestionConfidence)
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
                border.color: root.theme.border
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.preferredHeight: implicitHeight
                spacing: root.theme.spacingSmall

                Label {
                    color: root.theme.textPrimary
                    text: qsTr("Contract")
                    Layout.fillWidth: true
                    Layout.bottomMargin: -root.theme.spacingSmall
                }

                TransactionDraftFieldRow {
                    theme: root.theme
                    Layout.fillWidth: true
                    columnSpacing: root.splitColumnSpacing
                    leftLabel: ""
                    rightLabel: ""
                    leftWeight: root.contractMainLeftWeight
                    rightWeight: root.contractMainRightWeight

                    leftContent: Component {
                        ColumnLayout {
                            spacing: root.theme.spacingSmall

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: root.theme.spacingSmall
                                Label {
                                    color: root.theme.textPrimary
                                    text: qsTr("Name")
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: root.contractInnerNameWeight
                                }
                                Label {
                                    color: root.theme.textPrimary
                                    text: qsTr("Type")
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: root.contractInnerTypeWeight
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: root.theme.spacingSmall
                                Controls.TextField {
                                    objectName: "transactionDraftContractNameField"
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: root.contractInnerNameWeight
                                    placeholderText: root.importViewModel.contractNamePlaceholder
                                    text: root.importViewModel.contractName
                                    onTextEdited: root.importViewModel.contractName = text
                                    onEditingFinished: root.importViewModel.contractName = text
                                    onAccepted: root.importViewModel.contractName = text
                                    onActiveFocusChanged: if (!activeFocus)
                                        root.importViewModel.contractName = text
                                }
                                Controls.TextField {
                                    objectName: "transactionDraftContractTypeField"
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: root.contractInnerTypeWeight
                                    text: root.importViewModel.contractType
                                    onTextEdited: root.importViewModel.contractType = text
                                    onEditingFinished: root.importViewModel.contractType = text
                                    onAccepted: root.importViewModel.contractType = text
                                    onActiveFocusChanged: if (!activeFocus)
                                        root.importViewModel.contractType = text
                                }
                            }
                        }
                    }

                    rightContent: Component {
                        ColumnLayout {
                            spacing: root.theme.spacingSmall
                            Label {
                                color: root.theme.textPrimary
                                text: qsTr("Allocatable")
                                Layout.fillWidth: true
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: root.theme.spacingSmall

                                Controls.DropdownMenu {
                                    objectName: "transactionDraftContractAllocatableModeCombo"
                                    Layout.fillWidth: true
                                    textRole: "label"
                                    model: root.importViewModel.contractAllocatableModes
                                    currentIndex: root.importViewModel.contractAllocatableModeIndex
                                    onActivated: function (index) {
                                        root.importViewModel.contractAllocatableModeIndex = index;
                                    }
                                }

                                Controls.CompactAddButton {
                                    objectName: "transactionDraftContractAddButton"
                                    enabled: root.importViewModel.canAddContract
                                    onClicked: root.importViewModel.addContract()
                                }
                            }
                        }
                    }
                }

                TransactionDraftFieldRow {
                    theme: root.theme
                    Layout.fillWidth: true
                    columnSpacing: root.splitColumnSpacing
                    leftLabel: ""
                    rightLabel: ""
                    leftWeight: root.contractMainLeftWeight
                    rightWeight: root.contractMainRightWeight

                    leftContent: Component {
                        ColumnLayout {
                            spacing: root.theme.spacingSmall

                            Controls.Panel {
                                Layout.fillWidth: true
                                background: Rectangle {
                                    radius: root.theme.radius
                                    color: root.theme.surfaceAlt
                                    border.width: 1
                                    border.color: root.suggestionColor
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: root.theme.spacingSmall

                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: root.theme.spacingSmall

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            Layout.preferredWidth: root.contractInnerNameWeight
                                            spacing: root.theme.spacingSmall
                                            Label {
                                                color: root.theme.textPrimary
                                                text: qsTr("Select Contract")
                                                Layout.fillWidth: true
                                            }
                                            Controls.DropdownMenu {
                                                objectName: "transactionDraftContractCombo"
                                                Layout.fillWidth: true
                                                textRole: "display"
                                                model: root.importViewModel.contractOptions
                                                currentIndex: root.importViewModel.selectedContractOptionIndex
                                                onActivated: function (index) {
                                                    root.importViewModel.selectContractAtIndex(index);
                                                }
                                            }
                                        }

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            Layout.preferredWidth: root.contractInnerTypeWeight
                                            spacing: root.theme.spacingSmall
                                            Label {
                                                color: root.theme.textPrimary
                                                text: qsTr("Type")
                                                Layout.fillWidth: true
                                            }
                                            Controls.TextField {
                                                objectName: "transactionDraftContractSelectedTypeField"
                                                Layout.fillWidth: true
                                                readOnly: true
                                                text: root.importViewModel.selectedContractType
                                                color: root.theme.textMuted
                                                background: Rectangle {
                                                    radius: root.theme.radius
                                                    color: root.theme.surface
                                                    border.width: 1
                                                    border.color: root.theme.borderSoft
                                                    opacity: 0.7
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            Label {
                                objectName: "transactionDraftContractSuggestionLabel"
                                text: root.importViewModel.contractSuggestionSummary
                                color: root.suggestionColor
                                Layout.fillWidth: true
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.preferredHeight: root.theme.spacingSmall
                            }
                        }
                    }

                    rightContent: Component {
                        Import.TransactionDraftContractAllocatablePanel {
                            importViewModel: root.importViewModel
                            theme: root.theme
                        }
                    }
                }

                Import.TransactionDraftContractActorPanel {
                    importViewModel: root.importViewModel
                    theme: root.theme
                    embedded: true
                }

                Import.TransactionDraftContractPropertyPanel {
                    importViewModel: root.importViewModel
                    theme: root.theme
                    embedded: true
                }
            }
        }
    }
}
