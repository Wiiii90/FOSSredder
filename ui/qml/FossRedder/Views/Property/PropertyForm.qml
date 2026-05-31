/**
 * @file ui/qml/FossRedder/Views/Property/PropertyForm.qml
 * @brief Handles property create/update/delete flows and related form state.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Property 1.0 as Property
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var propertyState
    required property var theme

    readonly property bool isEdit: root.propertyState ? root.propertyState.isEdit : false
    readonly property var aliases: root.propertyState ? root.propertyState.aliases : []
    readonly property int aliasIndex: root.propertyState ? root.propertyState.aliasIndex : -1
    readonly property var selectedContractIds: root.propertyState ? root.propertyState.selectedContractIds : []
    readonly property var contractRows: root.propertyState.contractRows

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageContentMargin
        anchors.bottomMargin: 0
        spacing: root.theme.spacingSmall

        Flickable {
            id: propertyScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: propertyContent.height
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: propertyContent
                width: propertyScroll.width
                height: Math.max(implicitHeight, propertyScroll.height)
                spacing: root.theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: qsTr("Property Name")
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }

                    Controls.TextField {
                        id: nameField
                        objectName: "propertyNameField"
                        placeholderText: ""
                        Layout.fillWidth: true
                        text: root.propertyState ? root.propertyState.name : ""
                        onTextChanged: if (root.propertyState) root.propertyState.name = text
                        onTextEdited: if (root.propertyState) root.propertyState.name = text
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
                            id: propertyAliasInput
                            objectName: "propertyAliasInput"
                            Layout.fillWidth: true
                            placeholderText: ""
                            text: root.propertyState ? root.propertyState.aliasInputText : ""
                            onTextChanged: if (root.propertyState) root.propertyState.aliasInputText = text
                            onTextEdited: if (root.propertyState) root.propertyState.aliasInputText = text
                        }

                        Controls.CompactAddButton {
                            objectName: "propertyAddAliasButton"
                            enabled: root.propertyState ? root.propertyState.canAddAlias(propertyAliasInput.text) : false
                            onClicked: if (root.propertyState) root.propertyState.addAlias(propertyAliasInput.text)
                        }

                        Controls.CompactRemoveButton {
                            objectName: "propertyRemoveAliasButton"
                            enabled: root.aliasIndex >= 0 && root.aliasIndex < root.aliases.length
                            onClicked: if (root.propertyState) root.propertyState.requestRemoveSelectedAlias()
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
                            id: propertyAliasScroll
                            objectName: "propertyAliasScroll"
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            contentWidth: width
                            contentHeight: propertyAliasFlow.implicitHeight

                            ScrollBar.vertical: ScrollBar {
                                policy: ScrollBar.AsNeeded
                            }

                            Flow {
                                id: propertyAliasFlow
                                width: propertyAliasScroll.width
                                spacing: root.theme.spacingSmall

                                Repeater {
                                    model: root.aliases

                                    delegate: Rectangle {
                                        id: propertyAliasChip
                                        objectName: "propertyAliasChip_" + propertyAliasChip.index
                                        required property var modelData
                                        required property int index
                                        height: root.theme.viewAliasChipHeight
                                        radius: root.theme.viewAliasChipRadius
                                        color: root.aliasIndex === propertyAliasChip.index ? root.theme.selectionHighlight : root.theme.surfaceAlt
                                        border.width: 1
                                        border.color: root.theme.border
                                        width: Math.min(propertyAliasFlow.width, propertyAliasText.implicitWidth + root.theme.spacingLarge)

                                        Text {
                                            id: propertyAliasText
                                            anchors.centerIn: parent
                                            text: String(propertyAliasChip.modelData)
                                            color: root.theme.textPrimary
                                            elide: Text.ElideRight
                                            width: parent.width - root.theme.spacing
                                            horizontalAlignment: Text.AlignHCenter
                                        }

                                        MouseArea {
                                            objectName: "propertyAliasMouse_" + propertyAliasChip.index
                                            anchors.fill: parent
                                            preventStealing: true
                                            onPressed: if (root.propertyState) root.propertyState.aliasIndex = propertyAliasChip.index
                                            onClicked: if (root.propertyState) root.propertyState.aliasIndex = propertyAliasChip.index
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Property.PropertyContractPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight
                    Layout.preferredHeight: root.theme.viewSelectionPanelPreferredHeight
                    theme: root.theme
                    propertyState: root.propertyState
                    contractRows: root.contractRows
                }
            }
        }
    }
}
