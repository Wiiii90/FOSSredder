/**
 * @file ui/qml/FossRedder/Views/Actor/ActorForm.qml
 * @brief Provides the ActorForm component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Actor 1.0 as Actor

Item {
    id: root
    required property var actorState
    required property var theme

    readonly property var aliases: root.actorState ? root.actorState.aliases : []
    readonly property int aliasIndex: root.actorState ? root.actorState.aliasIndex : -1
    readonly property var contractRows: root.actorState.contractRows

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageContentMargin
        anchors.bottomMargin: 0
        spacing: root.theme.spacingSmall

        Flickable {
            id: actorScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: actorContent.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Controls.AppScrollBar { hidden: true }

            ColumnLayout {
                id: actorContent
                width: actorScroll.width
                spacing: root.theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        color: root.theme.textPrimary
                        text: qsTr("Actor Name")
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }

                    Controls.TextField {
                        id: nameField
                        objectName: "actorNameField"
                        placeholderText: ""
                        text: root.actorState ? root.actorState.name : ""
                        Layout.fillWidth: true
                        onTextChanged: if (root.actorState)
                            root.actorState.name = text
                        onTextEdited: if (root.actorState)
                            root.actorState.name = text
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
                            id: actorAliasInput
                            objectName: "actorAliasInput"
                            Layout.fillWidth: true
                            placeholderText: ""
                            text: root.actorState ? root.actorState.aliasInputText : ""
                            onTextChanged: if (root.actorState)
                                root.actorState.aliasInputText = text
                            onTextEdited: if (root.actorState)
                                root.actorState.aliasInputText = text
                        }

                        Controls.CompactAddButton {
                            objectName: "actorAddAliasButton"
                            enabled: root.actorState ? root.actorState.canAddAlias(actorAliasInput.text) : false
                            onClicked: if (root.actorState)
                                root.actorState.addAlias(actorAliasInput.text)
                        }

                        Controls.CompactRemoveButton {
                            objectName: "actorRemoveAliasButton"
                            enabled: root.aliasIndex >= 0 && root.aliasIndex < root.aliases.length
                            onClicked: if (root.actorState)
                                root.actorState.requestRemoveSelectedAlias()
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
                            id: actorAliasScroll
                            objectName: "actorAliasScroll"
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            contentWidth: width
                            contentHeight: actorAliasFlow.implicitHeight

                            ScrollBar.vertical: Controls.AppScrollBar {}

                            Flow {
                                id: actorAliasFlow
                                width: actorAliasScroll.width
                                spacing: root.theme.spacingSmall

                                Repeater {
                                    model: root.aliases

                                    delegate: Rectangle {
                                        id: actorAliasChip
                                        objectName: "actorAliasChip_" + actorAliasChip.index
                                        required property var modelData
                                        required property int index
                                        height: root.theme.viewAliasChipHeight
                                        radius: root.theme.viewAliasChipRadius
                                        color: root.aliasIndex === actorAliasChip.index ? root.theme.selectionHighlight : root.theme.surfaceAlt
                                        border.width: 1
                                        border.color: root.theme.border
                                        width: Math.min(actorAliasFlow.width, actorAliasText.implicitWidth + root.theme.spacingLarge)

                                        Text {
                                            id: actorAliasText
                                            anchors.centerIn: parent
                                            text: String(actorAliasChip.modelData)
                                            color: root.theme.textPrimary
                                            elide: Text.ElideRight
                                            width: parent.width - root.theme.spacing
                                            horizontalAlignment: Text.AlignHCenter
                                        }

                                        MouseArea {
                                            objectName: "actorAliasMouse_" + actorAliasChip.index
                                            anchors.fill: parent
                                            preventStealing: true
                                            onPressed: if (root.actorState)
                                                root.actorState.aliasIndex = actorAliasChip.index
                                            onClicked: if (root.actorState)
                                                root.actorState.aliasIndex = actorAliasChip.index
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Actor.ActorContractPanel {
                    Layout.fillWidth: true
                    theme: root.theme
                    actorState: root.actorState
                    contractRows: root.contractRows
                }
            }
        }
    }
}
