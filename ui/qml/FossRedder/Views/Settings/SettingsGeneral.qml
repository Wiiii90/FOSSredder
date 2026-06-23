/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsGeneral.qml
 * @brief Manages general settings options such as application language.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import FossRedder.Controls 1.0 as Controls

Flickable {
    id: root
    required property var settingsViewModel
    required property var theme
    Layout.fillWidth: true
    Layout.fillHeight: true
    contentHeight: column.implicitHeight
    contentWidth: width
    clip: true

    ScrollBar.vertical: Controls.AppScrollBar { hidden: true }

    ColumnLayout {
        id: column
        anchors.fill: parent
        width: parent.width
        spacing: root.theme.spacingSmall

        Controls.Panel {
            Layout.fillWidth: true
            contentSpacing: root.theme.spacingSmall

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Theme")
                        color: root.theme.textPrimary
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }
                    Controls.DropdownMenu {
                        id: themeMode
                        objectName: "settingsThemeModeDropdown"
                        model: root.settingsViewModel.themeModeOptions
                        textRole: "label"
                        currentIndex: root.settingsViewModel.themeModeIndex
                        onActivated: function (index) {
                            root.settingsViewModel.selectThemeModeAt(index);
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Language")
                        color: root.theme.textPrimary
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }
                    Controls.DropdownMenu {
                        id: language
                        objectName: "settingsLanguageDropdown"
                        model: root.settingsViewModel.languageOptions
                        textRole: "label"
                        currentIndex: root.settingsViewModel.languageIndex
                        onActivated: function (index) {
                            root.settingsViewModel.selectLanguageAt(index);
                        }
                    }
                }
            }
        }

        Controls.Panel {
            Layout.fillWidth: true
            contentSpacing: root.theme.spacingSmall

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                Text {
                    text: qsTr("Workspace")
                    color: root.theme.textPrimary
                    font.bold: true
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Save on close")
                        color: root.theme.textPrimary
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }
                    Controls.CheckBox {
                        objectName: "settingsAutosaveOnCloseCheckBox"
                        checked: root.settingsViewModel.autosaveOnClose
                        onToggled: root.settingsViewModel.autosaveOnClose = checked
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Autosave interval")
                        color: root.theme.textPrimary
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }
                    Controls.DropdownMenu {
                        objectName: "settingsAutosaveIntervalDropdown"
                        model: root.settingsViewModel.autosaveIntervalOptions
                        textRole: "label"
                        currentIndex: root.settingsViewModel.autosaveIntervalIndex
                        onActivated: function (index) {
                            root.settingsViewModel.selectAutosaveIntervalAt(index);
                        }
                    }
                }
            }
        }
    }
}
