/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsMiscellaneous.qml
 * @brief Provides miscellaneous settings toggles such as toolbar visibility options.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

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
            Layout.fillHeight: false
            Layout.preferredHeight: implicitHeight
            contentSpacing: root.theme.spacingSmall

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                Layout.preferredHeight: implicitHeight
                spacing: root.theme.spacingSmall

                GridLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                    columns: 2
                    columnSpacing: root.theme.spacingLarge
                    rowSpacing: root.theme.spacingSmall

                    Controls.CheckBox {
                        objectName: "settingsToolbarBookingCheckBox"
                        text: qsTr("Booking")
                        checked: root.settingsViewModel.toolbarShowBooking
                        onToggled: root.settingsViewModel.toolbarShowBooking = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarImportCheckBox"
                        text: qsTr("Import")
                        checked: root.settingsViewModel.toolbarShowImport
                        onToggled: root.settingsViewModel.toolbarShowImport = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarActorsCheckBox"
                        text: qsTr("Actors")
                        checked: root.settingsViewModel.toolbarShowActors
                        onToggled: root.settingsViewModel.toolbarShowActors = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarExportCheckBox"
                        text: qsTr("Export")
                        checked: root.settingsViewModel.toolbarShowExport
                        onToggled: root.settingsViewModel.toolbarShowExport = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarPropertiesCheckBox"
                        text: qsTr("Properties")
                        checked: root.settingsViewModel.toolbarShowProperties
                        onToggled: root.settingsViewModel.toolbarShowProperties = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarAnalysisCheckBox"
                        text: qsTr("Analysis")
                        checked: root.settingsViewModel.toolbarShowAnalysis
                        onToggled: root.settingsViewModel.toolbarShowAnalysis = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarContractsCheckBox"
                        text: qsTr("Contracts")
                        checked: root.settingsViewModel.toolbarShowContracts
                        onToggled: root.settingsViewModel.toolbarShowContracts = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarAnnualCheckBox"
                        text: qsTr("Annual")
                        checked: root.settingsViewModel.toolbarShowAnnual
                        onToggled: root.settingsViewModel.toolbarShowAnnual = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarSettingsCheckBox"
                        text: qsTr("Settings")
                        checked: root.settingsViewModel.toolbarShowSettings
                        onToggled: root.settingsViewModel.toolbarShowSettings = checked
                    }
                }
            }
        }
    }
}
