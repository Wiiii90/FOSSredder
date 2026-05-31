/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsView.qml
 * @brief Provides the SettingsView component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Settings 1.0 as Settings
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var settingsState
    required property var theme

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageContentMargin
        spacing: root.theme.spacingSmall

        StackLayout {
            id: settingsStack
            objectName: "settingsLoader"
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.settingsState.currentCategory

            Settings.SettingsGeneral {
                theme: root.theme
                settingsState: root.settingsState
            }

            Settings.SettingsImport {
                theme: root.theme
                settingsState: root.settingsState
            }

            Settings.SettingsExport {
                theme: root.theme
                settingsState: root.settingsState
            }

            Settings.SettingsMiscellaneous {
                theme: root.theme
                settingsState: root.settingsState
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.PrevButton {
                objectName: "settingsPrevCategoryButton"
                enabled: root.settingsState.canNavigateCategories
                onClicked: root.settingsState.navigateCategory(-1)
            }

            Item { Layout.fillWidth: true }

            Controls.DangerButton {
                objectName: "settingsDefaultButton"
                text: qsTr("Default")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.settingsState.resetSettings()
            }

            Controls.SuccessButton {
                objectName: "settingsUpdateButton"
                text: qsTr("Update")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.settingsState.saveSettings()
            }

            Item { Layout.fillWidth: true }

            Controls.NextButton {
                objectName: "settingsNextCategoryButton"
                enabled: root.settingsState.canNavigateCategories
                onClicked: root.settingsState.navigateCategory(1)
            }
        }
    }

    Component.onCompleted: root.settingsState.activate()
}
