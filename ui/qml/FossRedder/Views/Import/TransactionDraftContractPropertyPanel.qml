/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftContractPropertyPanel.qml
 * @brief Manages property selection inside the transaction draft contract panel.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var importViewModel
    required property var theme
    property bool embedded: false

    readonly property int suggestionTone: root.importViewModel.suggestionTone(root.importViewModel.propertySuggestionConfidence)
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
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                Layout.preferredHeight: implicitHeight
                spacing: root.theme.spacingSmall

                Label {
                    color: root.theme.textPrimary
                    text: qsTr("Select Property")
                    Layout.fillWidth: true
                }

                Controls.CheckListPanel {

                    Repeater {
                        id: propertyRepeater
                        model: root.importViewModel.propertyOptions

                        delegate: RowLayout {
                            id: propertyOption
                            required property var modelData
                            Layout.fillWidth: true
                            spacing: root.theme.spacingSmall

                            Controls.CheckBox {
                                objectName: "transactionDraftPropertyCheck_" + String(propertyOption.modelData.id || "")
                                Layout.fillWidth: false
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                checked: root.importViewModel.isPropertySelected(propertyOption.modelData.id)
                                onToggled: root.importViewModel.setPropertySelected(propertyOption.modelData.id, checked)
                            }

                            Label {
                                color: root.theme.textPrimary
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                text: propertyOption.modelData.display || propertyOption.modelData.name || ""
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                            }

                            Item {
                                Layout.fillWidth: true
                            }
                        }
                    }

                    Label {
                        visible: propertyRepeater.count === 0
                        text: qsTr("No properties available")
                        color: root.theme.textMuted
                        Layout.fillWidth: true
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: root.theme.spacingSmall

                        Label {
                            color: root.theme.textPrimary
                            text: qsTr("Name")
                            Layout.fillWidth: true
                        }

                        Controls.TextField {
                            objectName: "transactionDraftPropertyNameInput"
                            Layout.fillWidth: true
                            placeholderText: ""
                            text: root.importViewModel.propertyName
                            onTextEdited: root.importViewModel.propertyName = text
                            onAccepted: root.importViewModel.addProperty()
                        }
                    }

                    ColumnLayout {
                        spacing: root.theme.spacingSmall
                        Label {
                            color: root.theme.textPrimary
                            text: " "
                            Layout.fillWidth: false
                        }
                        Controls.CompactAddButton {
                            objectName: "transactionDraftPropertyAddButton"
                            enabled: root.importViewModel.canAddProperty
                            onClicked: root.importViewModel.addProperty()
                        }
                    }
                }
            }
        }

        Label {
            objectName: "transactionDraftPropertySuggestionLabel"
            text: root.importViewModel.propertySuggestionSummary
            color: root.suggestionColor
            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.theme.spacingSmall
        }
    }
}
