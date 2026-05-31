/**
 * @file ui/qml/FossRedder/Views/Contract/ContractPropertiesPanel.qml
 * @brief Provides the ContractPropertiesPanel component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var contractState
    property var propertyRows: []
    readonly property var selectedPropertyIds: root.contractState ? root.contractState.selectedPropertyIds : []

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
            text: qsTr("Properties")
            Layout.fillWidth: true
        }

        Controls.CheckListPanel {
            Repeater {
                model: root.propertyRows

                delegate: RowLayout {
                    id: propertyRow
                    required property var modelData
                    readonly property string propertyId: propertyRow.modelData && propertyRow.modelData.id ? propertyRow.modelData.id : ""

                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall

                    Controls.CheckBox {
                        objectName: "contractPropertyCheckBox"
                        Layout.fillWidth: false
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        checked: root.selectedPropertyIds.indexOf(propertyRow.propertyId) !== -1
                        onToggled: if (root.contractState)
                            root.contractState.setPropertySelected(propertyRow.propertyId, checked)
                    }

                    Label {
                        color: root.theme.textPrimary
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        text: propertyRow.modelData && propertyRow.modelData.name ? propertyRow.modelData.name : ""
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
