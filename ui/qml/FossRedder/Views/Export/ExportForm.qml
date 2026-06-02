/**
 * @file ui/qml/FossRedder/Views/Export/ExportForm.qml
 * @brief Provides the Export form component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var exportViewModel

    contentSpacing: root.theme.spacingSmall

    RowLayout {
        Layout.fillWidth: true

        Label {
            color: root.theme.textPrimary
            text: qsTr("Target")
            Layout.preferredWidth: root.theme.formLabelWidth
        }

        Controls.TextField {
            objectName: "exportTargetDirectoryField"
            Layout.fillWidth: true
            placeholderText: qsTr("Select target directory...")
            text: root.exportViewModel.targetDirectory
            onTextChanged: root.exportViewModel.targetDirectory = text
        }

        Controls.SecondaryButton {
            objectName: "exportBrowseDirectoryButton"
            text: qsTr("Browse...")
            onClicked: root.exportViewModel.browseDirectory()
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Label {
            color: root.theme.textPrimary
            text: qsTr("Archive")
            Layout.preferredWidth: root.theme.formLabelWidth
        }

        Controls.DropdownMenu {
            objectName: "exportArchiveFormatComboBox"
            Layout.fillWidth: true
            model: [qsTr("None"), qsTr("ZIP")]
            currentIndex: root.exportViewModel.packageFormatIndex
            onCurrentIndexChanged: root.exportViewModel.packageFormatIndex = currentIndex
        }
    }
}
