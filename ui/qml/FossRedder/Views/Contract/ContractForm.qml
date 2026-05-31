/**
 * @file ui/qml/FossRedder/Views/Contract/ContractForm.qml
 * @brief Provides the ContractForm component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Contract 1.0 as Contract
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var contractState
    required property var theme

    readonly property var aliases: root.contractState ? root.contractState.aliases : []
    readonly property int aliasIndex: root.contractState ? root.contractState.aliasIndex : -1
    readonly property var selectedActorIds: root.contractState ? root.contractState.selectedActorIds : []
    readonly property var selectedPropertyIds: root.contractState ? root.contractState.selectedPropertyIds : []
    readonly property var propertyRows: root.contractState.propertyRows

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageContentMargin
        anchors.bottomMargin: 0
        spacing: root.theme.spacingSmall

        Flickable {
            id: contractScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: contractContent.height
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: contractContent
                width: contractScroll.width
                height: Math.max(implicitHeight, contractScroll.height)
                spacing: root.theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: qsTr("Contract Name")
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }

                    Controls.TextField {
                        id: nameField
                        objectName: "contractNameField"
                        placeholderText: ""
                        Layout.fillWidth: true
                        text: root.contractState ? root.contractState.name : ""
                        onTextChanged: if (root.contractState) root.contractState.name = text
                        onTextEdited: if (root.contractState) root.contractState.name = text
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: root.theme.viewAliasGroupSpacing

                    RowLayout {
                        id: aliasControlsRow
                        Layout.fillWidth: true

                        Label {
                            text: qsTr("Aliases")
                            Layout.preferredWidth: root.theme.formLabelWidth
                        }

                        Controls.TextField {
                            id: contractAliasInput
                            objectName: "contractAliasInput"
                            Layout.fillWidth: true
                            placeholderText: ""
                            text: root.contractState ? root.contractState.aliasInputText : ""
                            onTextChanged: if (root.contractState) root.contractState.aliasInputText = text
                            onTextEdited: if (root.contractState) root.contractState.aliasInputText = text
                        }

                        Controls.CompactAddButton {
                            objectName: "contractAddAliasButton"
                            enabled: root.contractState ? root.contractState.canAddAlias(contractAliasInput.text) : false
                            onClicked: if (root.contractState) root.contractState.addAlias(contractAliasInput.text)
                        }

                        Controls.CompactRemoveButton {
                            objectName: "contractRemoveAliasButton"
                            enabled: root.aliasIndex >= 0 && root.aliasIndex < root.aliases.length
                            onClicked: if (root.contractState) root.contractState.requestRemoveSelectedAlias()
                        }
                    }

                    Controls.Panel {
                        Layout.fillWidth: true
                        Layout.minimumHeight: root.theme.viewAliasPanelMinHeight
                        Layout.preferredHeight: root.theme.viewAliasPanelPreferredHeight
                        Layout.maximumHeight: root.theme.viewAliasPanelPreferredHeight
                        contentSpacing: 0
                        background: Rectangle {
                            radius: root.theme.radius
                            color: root.theme.surface
                            border.width: 1
                            border.color: root.theme.border
                        }

                        Flickable {
                            id: contractAliasScroll
                            objectName: "contractAliasScroll"
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            contentWidth: width
                            contentHeight: contractAliasFlow.implicitHeight

                            ScrollBar.vertical: ScrollBar {
                                policy: ScrollBar.AsNeeded
                            }

                            Flow {
                                id: contractAliasFlow
                                width: contractAliasScroll.width
                                spacing: root.theme.spacingSmall

                                Repeater {
                                    model: root.aliases

                                    delegate: Rectangle {
                                        id: contractAliasChip
                                        objectName: "contractAliasChip_" + contractAliasChip.index
                                        required property var modelData
                                        required property int index
                                        height: root.theme.viewAliasChipHeight
                                        radius: root.theme.viewAliasChipRadius
                                        color: root.aliasIndex === contractAliasChip.index ? root.theme.selectionHighlight : root.theme.surfaceAlt
                                        border.width: 1
                                        border.color: root.theme.border
                                        width: Math.min(contractAliasFlow.width, contractAliasText.implicitWidth + root.theme.spacingLarge)

                                        Text {
                                            id: contractAliasText
                                            anchors.centerIn: parent
                                            text: String(contractAliasChip.modelData)
                                            color: root.theme.textPrimary
                                            elide: Text.ElideRight
                                            width: parent.width - root.theme.spacing
                                            horizontalAlignment: Text.AlignHCenter
                                        }

                                        MouseArea {
                                            objectName: "contractAliasMouse_" + contractAliasChip.index
                                            anchors.fill: parent
                                            preventStealing: true
                                            onPressed: if (root.contractState) root.contractState.aliasIndex = contractAliasChip.index
                                            onClicked: if (root.contractState) root.contractState.aliasIndex = contractAliasChip.index
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Contract.ContractTypePanel {
                    Layout.fillWidth: true
                    theme: root.theme
                    contractState: root.contractState
                }

                Contract.ContractAllocatablePanel {
                    Layout.fillWidth: true
                    theme: root.theme
                    contractState: root.contractState
                }

                Contract.ContractActorsPanel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: implicitHeight
                    Layout.maximumHeight: implicitHeight
                    theme: root.theme
                    contractState: root.contractState
                }

                Contract.ContractPropertiesPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight
                    Layout.preferredHeight: root.theme.viewSelectionPanelPreferredHeight
                    theme: root.theme
                    contractState: root.contractState
                    propertyRows: root.propertyRows
                }
            }
        }
    }
}
