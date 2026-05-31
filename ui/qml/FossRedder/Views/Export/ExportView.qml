/**
 * @file ui/qml/FossRedder/Views/Export/ExportView.qml
 * @brief Provides the Export view composition.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Export 1.0 as Export
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var exportState
    required property var theme

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageContentMargin
        spacing: root.theme.spacingSmall

        Flickable {
            id: exportScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: Math.max(exportContent.implicitHeight, exportScroll.height)

            ScrollBar.vertical: Controls.AppScrollBar { hidden: true }

            ColumnLayout {
                id: exportContent
                width: exportScroll.width
                height: Math.max(implicitHeight, exportScroll.height)
                spacing: root.theme.spacingSmall

                Export.ExportForm {
                    objectName: "exportFormPanel"
                    Layout.fillWidth: true
                    theme: root.theme
                    exportState: root.exportState
                }

                Export.ExportPanel {
                    objectName: "exportObjectsPanel"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: root.theme.exportView.panel.panelMinHeight
                    theme: root.theme
                    exportState: root.exportState
                }
            }
        }

        Export.ExportProgressBar {
            Layout.fillWidth: true
            theme: root.theme
            exportState: root.exportState
        }

        Export.ExportBottomBar {
            Layout.fillWidth: true
            theme: root.theme
            exportState: root.exportState
        }
    }

    Component.onCompleted: root.exportState.refreshFromWorkspace()
}
