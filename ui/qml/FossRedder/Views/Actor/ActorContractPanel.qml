/**
 * @file ui/qml/FossRedder/Views/Actor/ActorContractPanel.qml
 * @brief Provides the ActorContractPanel component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var actorState
    property var contractRows: []
    readonly property var selectedContractIds: root.actorState ? root.actorState.selectedContractIds : []

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
        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
        Layout.preferredHeight: implicitHeight
        spacing: root.theme.spacingSmall

        Label {
            color: root.theme.textPrimary
            text: qsTr("Contracts")
            Layout.fillWidth: true
        }

        Controls.CheckListPanel {
            Repeater {
                model: root.contractRows

                delegate: RowLayout {
                    id: contractRow
                    required property var modelData
                    readonly property string contractId: contractRow.modelData && contractRow.modelData.id ? contractRow.modelData.id : ""

                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall

                    Controls.CheckBox {
                        objectName: "actorContractCheckBox"
                        Layout.fillWidth: false
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        checked: root.selectedContractIds.indexOf(contractRow.contractId) !== -1
                        onToggled: if (root.actorState)
                            root.actorState.setContractSelected(contractRow.contractId, checked)
                    }

                    Label {
                        color: root.theme.textPrimary
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        text: contractRow.modelData && contractRow.modelData.name ? contractRow.modelData.name : ""
                        elide: Text.ElideRight
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
