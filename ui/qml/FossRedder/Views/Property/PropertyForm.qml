/**
 * @file ui/qml/FossRedder/Views/Property/PropertyForm.qml
 * @brief Handles property create/update/delete flows and related form state.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Property 1.0 as Property

Item {
    id: root
    required property var propertyViewModel
    required property var theme

    readonly property bool isEdit: root.propertyViewModel ? root.propertyViewModel.isEdit : false
    readonly property var aliases: root.propertyViewModel ? root.propertyViewModel.aliases : []
    readonly property int aliasIndex: root.propertyViewModel ? root.propertyViewModel.aliasIndex : -1
    readonly property var selectedContractIds: root.propertyViewModel ? root.propertyViewModel.selectedContractIds : []
    readonly property var contractRows: root.propertyViewModel.contractRows

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
            contentHeight: propertyContent.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Controls.AppScrollBar { hidden: true }

            ColumnLayout {
                id: propertyContent
                width: propertyScroll.width
                spacing: root.theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        color: root.theme.textPrimary
                        text: qsTr("Property Name")
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }

                    Controls.TextField {
                        id: nameField
                        objectName: "propertyNameField"
                        placeholderText: ""
                        Layout.fillWidth: true
                        text: root.propertyViewModel ? root.propertyViewModel.name : ""
                        onTextChanged: if (root.propertyViewModel)
                            root.propertyViewModel.name = text
                        onTextEdited: if (root.propertyViewModel)
                            root.propertyViewModel.name = text
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
                            id: propertyAliasInput
                            objectName: "propertyAliasInput"
                            Layout.fillWidth: true
                            placeholderText: ""
                            text: root.propertyViewModel ? root.propertyViewModel.aliasInputText : ""
                            onTextChanged: if (root.propertyViewModel)
                                root.propertyViewModel.aliasInputText = text
                            onTextEdited: if (root.propertyViewModel)
                                root.propertyViewModel.aliasInputText = text
                        }

                        Controls.CompactAddButton {
                            objectName: "propertyAddAliasButton"
                            enabled: root.propertyViewModel ? root.propertyViewModel.canAddAlias(propertyAliasInput.text) : false
                            onClicked: if (root.propertyViewModel)
                                root.propertyViewModel.addAlias(propertyAliasInput.text)
                        }

                        Controls.CompactRemoveButton {
                            objectName: "propertyRemoveAliasButton"
                            enabled: root.aliasIndex >= 0 && root.aliasIndex < root.aliases.length
                            onClicked: if (root.propertyViewModel)
                                root.propertyViewModel.requestRemoveSelectedAlias()
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

                            ScrollBar.vertical: Controls.AppScrollBar {}

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
                                            onPressed: if (root.propertyViewModel)
                                                root.propertyViewModel.aliasIndex = propertyAliasChip.index
                                            onClicked: if (root.propertyViewModel)
                                                root.propertyViewModel.aliasIndex = propertyAliasChip.index
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Property.PropertyContractPanel {
                    Layout.fillWidth: true
                    theme: root.theme
                    propertyViewModel: root.propertyViewModel
                    contractRows: root.contractRows
                }
            }
        }
    }
}
