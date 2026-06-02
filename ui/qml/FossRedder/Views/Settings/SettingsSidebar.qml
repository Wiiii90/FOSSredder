/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsSidebar.qml
 * @brief Renders settings category navigation entries in the sidebar.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var settingsViewModel
    required property var theme

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Flickable {
            id: settingsSidebarFlick
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: settingsColumn.implicitHeight

            ScrollBar.vertical: Controls.AppScrollBar {
                parent: settingsSidebarFlick
                anchors.right: settingsSidebarFlick.right
                anchors.rightMargin: root.theme.viewSidebarScrollBarOuterInset
                anchors.top: settingsSidebarFlick.top
                anchors.bottom: settingsSidebarFlick.bottom
                persistent: true
            }

            Column {
                id: settingsColumn
                x: root.theme.viewSidebarEntryInset
                width: Math.max(0, parent.width - root.theme.viewSidebarEntryInsetTotal)
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.settingsViewModel.categoryRows

                    delegate: Rectangle {
                        id: settingsRow
                        required property var modelData
                        objectName: "settingsSidebarCategory_" + settingsRow.modelData.category
                        width: settingsColumn.width
                        height: root.theme.viewSidebarRowHeight
                        radius: root.theme.viewSidebarRowRadius
                        color: settingsRow.modelData.selected ? root.theme.selectionHighlight : (settingsMouse.containsMouse ? root.theme.sidebarHoverFill : "transparent")
                        border.color: settingsRow.modelData.selected ? root.theme.selectionBorder : (settingsMouse.containsMouse ? root.theme.sidebarHoverBorder : root.theme.borderSoft)
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            id: settingsMouse
                            objectName: "settingsSidebarCategoryClick_" + settingsRow.modelData.category
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.settingsViewModel.selectCategory(settingsRow.modelData.category)
                        }

                        Text {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.leftMargin: root.theme.spacingSmall
                            anchors.topMargin: root.theme.spacingSmall
                            width: parent.width - (2 * root.theme.spacingSmall)
                            text: settingsRow.modelData.text
                            color: root.theme.textPrimary
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }
}
