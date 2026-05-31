/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsMiscellaneous.qml
 * @brief Provides miscellaneous settings toggles such as toolbar visibility options.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Flickable {
    id: root
    required property var settingsState
    required property var theme
    Layout.fillWidth: true
    Layout.fillHeight: true
    contentHeight: column.implicitHeight
    contentWidth: width
    clip: true

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

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: root.theme.spacingLarge
                    rowSpacing: root.theme.spacingSmall

                    Controls.CheckBox {
                        objectName: "settingsToolbarBookingCheckBox"
                        text: qsTr("Booking")
                        checked: root.settingsState.toolbarShowBooking
                        onToggled: root.settingsState.toolbarShowBooking = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarImportCheckBox"
                        text: qsTr("Import")
                        checked: root.settingsState.toolbarShowImport
                        onToggled: root.settingsState.toolbarShowImport = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarActorsCheckBox"
                        text: qsTr("Actors")
                        checked: root.settingsState.toolbarShowActors
                        onToggled: root.settingsState.toolbarShowActors = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarExportCheckBox"
                        text: qsTr("Export")
                        checked: root.settingsState.toolbarShowExport
                        onToggled: root.settingsState.toolbarShowExport = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarPropertiesCheckBox"
                        text: qsTr("Properties")
                        checked: root.settingsState.toolbarShowProperties
                        onToggled: root.settingsState.toolbarShowProperties = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarAnalysisCheckBox"
                        text: qsTr("Analysis")
                        checked: root.settingsState.toolbarShowAnalysis
                        onToggled: root.settingsState.toolbarShowAnalysis = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarContractsCheckBox"
                        text: qsTr("Contracts")
                        checked: root.settingsState.toolbarShowContracts
                        onToggled: root.settingsState.toolbarShowContracts = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarAnnualCheckBox"
                        text: qsTr("Annual")
                        checked: root.settingsState.toolbarShowAnnual
                        onToggled: root.settingsState.toolbarShowAnnual = checked
                    }

                    Controls.CheckBox {
                        objectName: "settingsToolbarSettingsCheckBox"
                        text: qsTr("Settings")
                        checked: root.settingsState.toolbarShowSettings
                        onToggled: root.settingsState.toolbarShowSettings = checked
                    }
                }
            }
        }
    }
}
