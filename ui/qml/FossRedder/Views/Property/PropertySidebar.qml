/**
 * @file ui/qml/FossRedder/Views/Property/PropertySidebar.qml
 * @brief Shows and selects property entries in the sidebar list.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var propertyState
    required property var theme

    readonly property var propertyRows: root.propertyState.propertyRows

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Flickable {
            id: propertySidebarFlick
            objectName: "propertySidebarFlick"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: propertyColumn.implicitHeight

            ScrollBar.vertical: Controls.AppScrollBar {
                parent: propertySidebarFlick
                anchors.right: propertySidebarFlick.right
                anchors.rightMargin: root.theme.viewSidebarScrollBarOuterInset
                anchors.top: propertySidebarFlick.top
                anchors.bottom: propertySidebarFlick.bottom
                persistent: true
            }

            Column {
                id: propertyColumn
                x: root.theme.viewSidebarEntryInset
                width: Math.max(0, parent.width - root.theme.viewSidebarEntryInsetTotal)
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.propertyRows

                    delegate: Rectangle {
                        id: propertyRow
                        objectName: "propertySidebarRow_" + (propertyRow.modelData && propertyRow.modelData.id ? String(propertyRow.modelData.id) : "")
                        required property var modelData
                        readonly property string propertyId: propertyRow.modelData && propertyRow.modelData.id ? String(propertyRow.modelData.id) : ""
                        width: propertyColumn.width
                        height: root.theme.viewSidebarRowHeight
                        radius: root.theme.viewSidebarRowRadius
                        color: propertyRow.propertyId === root.propertyState.currentId ? root.theme.selectionHighlight : (propertyMouse.containsMouse ? root.theme.sidebarHoverFill : "transparent")
                        border.color: propertyRow.propertyId === root.propertyState.currentId ? root.theme.selectionBorder : (propertyMouse.containsMouse ? root.theme.sidebarHoverBorder : root.theme.borderSoft)
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            id: propertyMouse
                            objectName: "propertySidebarMouse_" + propertyRow.propertyId
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.propertyState.selectProperty(propertyRow.propertyId)
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: root.theme.viewSidebarRowSpacing

                            Text {
                                objectName: "propertySidebarName_" + propertyRow.propertyId
                                width: parent.width
                                text: propertyRow.modelData.name ? propertyRow.modelData.name : ""
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
