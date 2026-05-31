/**
 * @file ui/qml/FossRedder/Views/Import/ImportHomeView.qml
 * @brief Composes the import start page.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Import 1.0 as Import
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var importState

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Flickable {
            id: scroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentHeight: Math.max(content.implicitHeight, scroll.height)
            contentWidth: width

            ScrollBar.vertical: Controls.AppScrollBar { hidden: true }

            ColumnLayout {
                id: content
                width: scroll.width
                height: Math.max(implicitHeight, scroll.height)
                spacing: root.theme.spacingSmall

                Import.ImportForm {
                    Layout.fillWidth: true
                    theme: root.theme
                    importState: root.importState
                }

                Import.ImportPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 260
                    theme: root.theme
                    importState: root.importState
                }
            }
        }

        Import.ImportProgressBar {
            Layout.fillWidth: true
            theme: root.theme
            importState: root.importState
        }

        Import.ImportBottomBar {
            Layout.fillWidth: true
            theme: root.theme
            importState: root.importState
        }
    }
}
