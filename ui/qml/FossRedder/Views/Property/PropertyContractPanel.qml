/**
 * @file ui/qml/FossRedder/Views/Property/PropertyContractPanel.qml
 * @brief Provides contract selection controls used by the property form.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var propertyViewModel
    property var contractRows: []

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
                        objectName: "propertyContractCheckBox"
                        Layout.fillWidth: false
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        checked: root.propertyViewModel && root.propertyViewModel.selectedContractIds ? root.propertyViewModel.selectedContractIds.indexOf(contractRow.contractId) !== -1 : false
                        onToggled: if (root.propertyViewModel)
                            root.propertyViewModel.setContractSelected(contractRow.contractId, checked)
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
