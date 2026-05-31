/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsGeneral.qml
 * @brief Manages general settings options such as application language.
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
                        text: qsTr("Language")
                        color: root.theme.textPrimary
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }
                    Controls.DropdownMenu {
                        id: language
                        objectName: "settingsLanguageDropdown"
                        model: root.settingsState.languageOptions
                        textRole: "label"
                        currentIndex: root.settingsState.languageIndex
                        onActivated: function(index) {
                            root.settingsState.selectLanguageAt(index)
                        }
                    }
                }
            }
        }
    }
}
