/**
 * @file ui/qml/FossRedder/Views/Export/ExportBottomBar.qml
 * @brief Provides the Export action bar.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var exportViewModel

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: root
        theme: root.theme

        Controls.DangerButton {
            objectName: "exportClearButton"
            text: qsTr("Clear")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.exportViewModel.showClear
            enabled: root.exportViewModel.showClear
            onClicked: root.exportViewModel.clearForm()
        }

        Controls.DangerButton {
            objectName: "exportCancelButton"
            text: qsTr("Cancel")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.exportViewModel.showCancel
            enabled: root.exportViewModel.showCancel
            onClicked: root.exportViewModel.cancelExport()
        }

        Item { Layout.fillWidth: true }

        Controls.SuccessButton {
            objectName: "exportPauseResumeButton"
            text: root.exportViewModel.pauseText
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.exportViewModel.showPause
            enabled: root.exportViewModel.showPause
            onClicked: root.exportViewModel.isPaused
                       ? root.exportViewModel.resumeExport()
                       : root.exportViewModel.pauseExport()
        }

        Controls.SuccessButton {
            objectName: "exportStartButton"
            text: qsTr("Start")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.exportViewModel.showClear
            enabled: root.exportViewModel.canStart
            onClicked: root.exportViewModel.startExport()
        }
    }
}
