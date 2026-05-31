/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsImport.qml
 * @brief Configures import defaults and pipeline-related import settings.
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

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Default PDF file")
                        color: root.theme.textPrimary
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }
                    Controls.TextField {
                        id: defaultImportPathField
                        objectName: "settingsImportDefaultPathField"
                        Layout.fillWidth: true
                        placeholderText: qsTr("Select default PDF file...")
                        text: root.settingsState.importDefaultPath
                        onTextChanged: root.settingsState.importDefaultPath = text
                    }
                    Controls.SecondaryButton {
                        objectName: "settingsImportBrowseButton"
                        text: qsTr("Browse...")
                        Layout.preferredHeight: defaultImportPathField.implicitHeight
                        onClicked: root.settingsState.browseImportPath()
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
                    text: qsTr("Pipeline configuration")
                    color: root.theme.textPrimary
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Poppler"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.TextField {
                        objectName: "settingsImportPopplerField"
                        Layout.fillWidth: true
                        placeholderText: qsTr("Placeholder for Poppler defaults")
                        text: root.settingsState.importPoppler
                        onTextChanged: root.settingsState.importPoppler = text
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("OpenCV"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.TextField {
                        objectName: "settingsImportOpenCvField"
                        Layout.fillWidth: true
                        placeholderText: qsTr("Placeholder for OpenCV defaults")
                        text: root.settingsState.importOpenCv
                        onTextChanged: root.settingsState.importOpenCv = text
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Tesseract"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.TextField {
                        objectName: "settingsImportTesseractField"
                        Layout.fillWidth: true
                        placeholderText: qsTr("Placeholder for Tesseract defaults")
                        text: root.settingsState.importTesseract
                        onTextChanged: root.settingsState.importTesseract = text
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Parser"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.TextField {
                        objectName: "settingsImportParserField"
                        Layout.fillWidth: true
                        placeholderText: qsTr("Placeholder for parser defaults")
                        text: root.settingsState.importParser
                        onTextChanged: root.settingsState.importParser = text
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Matcher"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.TextField {
                        objectName: "settingsImportMatcherField"
                        Layout.fillWidth: true
                        placeholderText: qsTr("Placeholder for matcher defaults")
                        text: root.settingsState.importMatcher
                        onTextChanged: root.settingsState.importMatcher = text
                    }
                }
            }
        }
    }
}
