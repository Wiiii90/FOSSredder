/**
 * @file ui/qml/FossRedder/Views/Actor/ActorSidebar.qml
 * @brief Provides the ActorSidebar component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var actorState
    required property var theme

    readonly property var actorRows: root.actorState.actorRows

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Flickable {
            id: actorSidebarFlick
            objectName: "actorSidebarFlick"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: actorColumn.implicitHeight

            ScrollBar.vertical: Controls.AppScrollBar {
                parent: actorSidebarFlick
                anchors.right: actorSidebarFlick.right
                anchors.rightMargin: root.theme.viewSidebarScrollBarOuterInset
                anchors.top: actorSidebarFlick.top
                anchors.bottom: actorSidebarFlick.bottom
                persistent: true
            }

            Column {
                id: actorColumn
                x: root.theme.viewSidebarEntryInset
                width: Math.max(0, parent.width - root.theme.viewSidebarEntryInsetTotal)
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.actorRows

                    delegate: Rectangle {
                        id: actorRow
                        objectName: "actorSidebarRow_" + actorRow.actorId
                        required property var modelData
                        readonly property string actorId: actorRow.modelData && actorRow.modelData.id ? String(actorRow.modelData.id) : ""
                        width: actorColumn.width
                        height: root.theme.viewSidebarRowHeight
                        radius: root.theme.viewSidebarRowRadius
                        color: actorRow.actorId === root.actorState.currentId ? root.theme.selectionHighlight : (actorMouse.containsMouse ? root.theme.sidebarHoverFill : "transparent")
                        border.color: actorRow.actorId === root.actorState.currentId ? root.theme.selectionBorder : (actorMouse.containsMouse ? root.theme.sidebarHoverBorder : root.theme.borderSoft)
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            id: actorMouse
                            objectName: "actorSidebarMouse_" + actorRow.actorId
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            preventStealing: true
                            onClicked: root.actorState.selectActor(actorRow.actorId)
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: root.theme.viewSidebarRowSpacing

                            Text {
                                id: actorNameText
                                objectName: "actorSidebarName_" + actorRow.actorId
                                width: parent.width
                                text: actorRow.modelData.name ? actorRow.modelData.name : ""
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
