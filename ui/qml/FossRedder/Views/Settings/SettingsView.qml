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
    required property var settingsViewModel
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
            currentIndex: root.settingsViewModel.currentCategory

            Settings.SettingsGeneral {
                theme: root.theme
                settingsViewModel: root.settingsViewModel
            }

            Settings.SettingsImport {
                theme: root.theme
                settingsViewModel: root.settingsViewModel
            }

            Settings.SettingsExport {
                theme: root.theme
                settingsViewModel: root.settingsViewModel
            }

            Settings.SettingsMiscellaneous {
                theme: root.theme
                settingsViewModel: root.settingsViewModel
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.PrevButton {
                objectName: "settingsPrevCategoryButton"
                enabled: root.settingsViewModel.canNavigateCategories
                onClicked: root.settingsViewModel.navigateCategory(-1)
            }

            Item { Layout.fillWidth: true }

            Controls.DangerButton {
                objectName: "settingsDefaultButton"
                text: qsTr("Default")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.settingsViewModel.resetSettings()
            }

            Controls.SuccessButton {
                objectName: "settingsUpdateButton"
                text: qsTr("Update")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.settingsViewModel.saveSettings()
            }

            Item { Layout.fillWidth: true }

            Controls.NextButton {
                objectName: "settingsNextCategoryButton"
                enabled: root.settingsViewModel.canNavigateCategories
                onClicked: root.settingsViewModel.navigateCategory(1)
            }
        }
    }

    Component.onCompleted: root.settingsViewModel.activate()
}
