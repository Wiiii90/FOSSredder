/**
 * @file ui/qml/FossRedder/Views/Contract/ContractSidebar.qml
 * @brief Provides the ContractSidebar component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var contractState
    required property var theme

    readonly property var contractRows: root.contractState.contractRows

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacingSmall

        Flickable {
            objectName: "contractSidebarFlick"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: contractColumn.implicitHeight

            Column {
                id: contractColumn
                width: parent.width
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.contractRows

                    delegate: Rectangle {
                        id: contractRow
                        objectName: "contractSidebarRow_" + contractRow.contractId
                        required property var modelData
                        readonly property string contractId: contractRow.modelData && contractRow.modelData.id ? String(contractRow.modelData.id) : ""
                        width: contractColumn.width
                        height: root.theme.viewSidebarRowHeight
                        radius: root.theme.viewSidebarRowRadius
                        color: contractRow.contractId === root.contractState.currentId
                               ? root.theme.selectionHighlight
                               : "transparent"
                        border.color: root.theme.borderSoft
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            objectName: "contractSidebarMouse_" + contractRow.contractId
                            anchors.fill: parent
                            preventStealing: true
                            onClicked: root.contractState.selectContract(contractRow.contractId)
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: root.theme.viewSidebarRowSpacing

                            Text {
                                objectName: "contractSidebarName_" + contractRow.contractId
                                width: parent.width
                                text: contractRow.modelData.name ? contractRow.modelData.name : ""
                                color: root.theme.textPrimary
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }
    }
}
