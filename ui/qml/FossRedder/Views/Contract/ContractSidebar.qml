/**
 * @file ui/qml/FossRedder/Views/Contract/ContractSidebar.qml
 * @brief Provides the ContractSidebar component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var contractViewModel
    required property var theme

    readonly property var contractRows: root.contractViewModel.contractRows

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Flickable {
            id: contractSidebarFlick
            objectName: "contractSidebarFlick"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: contractColumn.implicitHeight

            ScrollBar.vertical: Controls.AppScrollBar {
                parent: contractSidebarFlick
                anchors.right: contractSidebarFlick.right
                anchors.rightMargin: root.theme.viewSidebarScrollBarOuterInset
                anchors.top: contractSidebarFlick.top
                anchors.bottom: contractSidebarFlick.bottom
                persistent: true
            }

            Column {
                id: contractColumn
                x: root.theme.viewSidebarEntryInset
                width: Math.max(0, parent.width - root.theme.viewSidebarEntryInsetTotal)
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
                        color: contractRow.contractId === root.contractViewModel.currentId ? root.theme.selectionHighlight : (contractMouse.containsMouse ? root.theme.sidebarHoverFill : "transparent")
                        border.color: contractRow.contractId === root.contractViewModel.currentId ? root.theme.selectionBorder : (contractMouse.containsMouse ? root.theme.sidebarHoverBorder : root.theme.borderSoft)
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            id: contractMouse
                            objectName: "contractSidebarMouse_" + contractRow.contractId
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            preventStealing: true
                            onClicked: root.contractViewModel.selectContract(contractRow.contractId)
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
