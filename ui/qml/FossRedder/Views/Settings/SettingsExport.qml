/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsExport.qml
 * @brief Edits export defaults such as output directory and archive format.
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

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Default output folder")
                        color: root.theme.textPrimary
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }
                    Controls.TextField {
                        objectName: "settingsExportDefaultDirectoryField"
                        Layout.fillWidth: true
                        placeholderText: qsTr("Select default output folder...")
                        text: root.settingsViewModel.exportDefaultDirectory
                        onTextChanged: root.settingsViewModel.exportDefaultDirectory = text
                    }
                    Controls.SecondaryButton {
                        objectName: "settingsExportBrowseButton"
                        text: qsTr("Browse...")
                        onClicked: root.settingsViewModel.browseExportDirectory()
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Default archive")
                        color: root.theme.textPrimary
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }
                    Controls.DropdownMenu {
                        objectName: "settingsExportArchiveFormatComboBox"
                        model: [qsTr("None"), qsTr("ZIP")]
                        currentIndex: root.settingsViewModel.exportArchiveFormat
                        onActivated: function(index) {
                            root.settingsViewModel.exportArchiveFormat = index
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("XLSX formulas")
                        color: root.theme.textPrimary
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }
                    Controls.CheckBox {
                        objectName: "settingsExportIncludeFormulasCheckBox"
                        checked: root.settingsViewModel.exportIncludeFormulas
                        text: qsTr("Use Excel formulas for totals when possible")
                        onToggled: root.settingsViewModel.exportIncludeFormulas = checked
                    }
                }
            }
        }
    }
}
