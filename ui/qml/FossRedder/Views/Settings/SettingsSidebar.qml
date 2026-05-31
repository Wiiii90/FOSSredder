/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsSidebar.qml
 * @brief Renders settings category navigation entries in the sidebar.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var settingsState
    required property var theme

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacingSmall

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: settingsColumn.implicitHeight

            Column {
                id: settingsColumn
                width: parent.width
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.settingsState.categoryRows

                    delegate: Rectangle {
                        id: settingsRow
                        required property var modelData
                        objectName: "settingsSidebarCategory_" + settingsRow.modelData.category
                        width: settingsColumn.width
                        height: root.theme.viewSidebarRowHeight
                        radius: root.theme.viewSidebarRowRadius
                        color: settingsRow.modelData.selected ? root.theme.selectionHighlight : "transparent"
                        border.color: root.theme.borderSoft
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            objectName: "settingsSidebarCategoryClick_" + settingsRow.modelData.category
                            anchors.fill: parent
                            onClicked: root.settingsState.selectCategory(settingsRow.modelData.category)
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
