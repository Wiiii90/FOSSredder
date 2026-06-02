/**
 * @file ui/qml/FossRedder/Views/Export/ExportProgressBar.qml
 * @brief Provides the Export progress component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var exportViewModel

    Layout.fillWidth: true
    contentSpacing: root.theme.spacingSmall

    Controls.ProgressBar {
        objectName: "exportProgressBar"
        Layout.fillWidth: true
        value: root.exportViewModel.progress
    }

    Label {
        objectName: "exportProgressStatusLabel"
        Layout.fillWidth: true
        text: root.exportViewModel.statusText
        color: root.exportViewModel.hasError ? root.theme.danger : root.theme.textPrimary
        wrapMode: Text.WordWrap
    }
}
