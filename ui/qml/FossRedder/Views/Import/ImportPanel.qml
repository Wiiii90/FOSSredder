/**
 * @file ui/qml/FossRedder/Views/Import/ImportPanel.qml
 * @brief Provides the import file selection panel.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var importViewModel

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.minimumHeight: 260
    contentSpacing: root.theme.spacingSmall

    RowLayout {
        Layout.fillWidth: true

        readonly property real actionButtonHeight: manualPath.implicitHeight

        Controls.TextField {
            id: manualPath
            objectName: "importManualPathField"
            Layout.fillWidth: true
            placeholderText: qsTr("Enter file path...")
            enabled: root.importViewModel.hasImportWorkflow && !root.importViewModel.importRunning
            text: root.importViewModel.manualPathText
            onTextEdited: root.importViewModel.manualPathText = text
        }

        Controls.AddButton {
            objectName: "importAddFileButton"
            Layout.preferredHeight: parent.actionButtonHeight
            enabled: root.importViewModel.hasImportWorkflow
                     && !root.importViewModel.importRunning
                     && root.importViewModel.manualPathText.trim().length > 0
            onClicked: root.importViewModel.addSelectedImportFiles()
        }

        Controls.SecondaryButton {
            objectName: "importBrowseFileButton"
            text: qsTr("Browse...")
            Layout.preferredHeight: parent.actionButtonHeight
            enabled: root.importViewModel.hasImportWorkflow && !root.importViewModel.importRunning
            onClicked: root.importViewModel.browseImportPdf()
        }
    }

    Controls.DropZone {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 160
        enabled: root.importViewModel.hasImportWorkflow && !root.importViewModel.importRunning
        title: qsTr("Drop PDFs here")
        subtitle: ""
        allowBrowse: false
        clickToBrowse: true
        queuedCount: root.importViewModel.queuedCount
        files: root.importViewModel.importFiles
        fileSummary: root.importViewModel.importFileSummary
        onBrowseRequested: root.importViewModel.browseImportPdf()
    }
}
