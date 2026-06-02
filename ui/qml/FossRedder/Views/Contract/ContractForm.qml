/**
 * @file ui/qml/FossRedder/Views/Contract/ContractForm.qml
 * @brief Provides the ContractForm component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Contract 1.0 as Contract

Item {
    id: root
    required property var contractViewModel
    required property var theme

    readonly property var aliases: root.contractViewModel ? root.contractViewModel.aliases : []
    readonly property int aliasIndex: root.contractViewModel ? root.contractViewModel.aliasIndex : -1
    readonly property var selectedActorIds: root.contractViewModel ? root.contractViewModel.selectedActorIds : []
    readonly property var selectedPropertyIds: root.contractViewModel ? root.contractViewModel.selectedPropertyIds : []
    readonly property var propertyRows: root.contractViewModel.propertyRows

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
            contentHeight: contractContent.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Controls.AppScrollBar { hidden: true }

            ColumnLayout {
                id: contractContent
                width: contractScroll.width
                spacing: root.theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        color: root.theme.textPrimary
                        text: qsTr("Contract Name")
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }

                    Controls.TextField {
                        id: nameField
                        objectName: "contractNameField"
                        placeholderText: ""
                        Layout.fillWidth: true
                        text: root.contractViewModel ? root.contractViewModel.name : ""
                        onTextChanged: if (root.contractViewModel)
                            root.contractViewModel.name = text
                        onTextEdited: if (root.contractViewModel)
                            root.contractViewModel.name = text
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: root.theme.viewAliasGroupSpacing

                    RowLayout {
                        id: aliasControlsRow
                        Layout.fillWidth: true

                        Label {
                            color: root.theme.textPrimary
                            text: qsTr("Aliases")
                            Layout.preferredWidth: root.theme.formLabelWidth
                        }

                        Controls.TextField {
                            id: contractAliasInput
                            objectName: "contractAliasInput"
                            Layout.fillWidth: true
                            placeholderText: ""
                            text: root.contractViewModel ? root.contractViewModel.aliasInputText : ""
                            onTextChanged: if (root.contractViewModel)
                                root.contractViewModel.aliasInputText = text
                            onTextEdited: if (root.contractViewModel)
                                root.contractViewModel.aliasInputText = text
                        }

                        Controls.CompactAddButton {
                            objectName: "contractAddAliasButton"
                            enabled: root.contractViewModel ? root.contractViewModel.canAddAlias(contractAliasInput.text) : false
                            onClicked: if (root.contractViewModel)
                                root.contractViewModel.addAlias(contractAliasInput.text)
                        }

                        Controls.CompactRemoveButton {
                            objectName: "contractRemoveAliasButton"
                            enabled: root.aliasIndex >= 0 && root.aliasIndex < root.aliases.length
                            onClicked: if (root.contractViewModel)
                                root.contractViewModel.requestRemoveSelectedAlias()
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

                            ScrollBar.vertical: Controls.AppScrollBar {}

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
                                            onPressed: if (root.contractViewModel)
                                                root.contractViewModel.aliasIndex = contractAliasChip.index
                                            onClicked: if (root.contractViewModel)
                                                root.contractViewModel.aliasIndex = contractAliasChip.index
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
                    contractViewModel: root.contractViewModel
                }

                Contract.ContractAllocatablePanel {
                    Layout.fillWidth: true
                    theme: root.theme
                    contractViewModel: root.contractViewModel
                }

                Contract.ContractActorsPanel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: implicitHeight
                    Layout.maximumHeight: implicitHeight
                    theme: root.theme
                    contractViewModel: root.contractViewModel
                }

                Contract.ContractPropertiesPanel {
                    Layout.fillWidth: true
                    theme: root.theme
                    contractViewModel: root.contractViewModel
                    propertyRows: root.propertyRows
                }
            }
        }
    }
}
