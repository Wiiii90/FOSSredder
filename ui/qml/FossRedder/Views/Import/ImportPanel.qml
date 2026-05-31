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
    required property var importState

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
            enabled: root.importState.hasImportWorkflow && !root.importState.importRunning
            text: root.importState.manualPathText
            onTextEdited: root.importState.manualPathText = text
        }

        Controls.AddButton {
            objectName: "importAddFileButton"
            Layout.preferredHeight: parent.actionButtonHeight
            enabled: root.importState.hasImportWorkflow
                     && !root.importState.importRunning
                     && root.importState.manualPathText.trim().length > 0
            onClicked: root.importState.commitManualImportFiles()
        }

        Controls.SecondaryButton {
            objectName: "importBrowseFileButton"
            text: qsTr("Browse...")
            Layout.preferredHeight: parent.actionButtonHeight
            enabled: root.importState.hasImportWorkflow && !root.importState.importRunning
            onClicked: root.importState.browseImportPdf()
        }
    }

    Controls.DropZone {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 160
        enabled: root.importState.hasImportWorkflow && !root.importState.importRunning
        title: qsTr("Drop PDFs here")
        subtitle: ""
        allowBrowse: false
        clickToBrowse: true
        queuedCount: root.importState.queuedCount
        files: root.importState.importFiles
        fileSummary: root.importState.importFileSummary
        onBrowseRequested: root.importState.browseImportPdf()
    }
}
