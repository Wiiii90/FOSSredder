/**
 * @file ui/qml/FossRedder/Views/Import/ImportBottomBar.qml
 * @brief Provides import overview bottom actions.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var importViewModel

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: root
        theme: root.theme

        Controls.PrevPageButton {
            objectName: "importPreviousDraftButton"
            enabled: root.importViewModel.hasDraftNavigation
            onClicked: root.importViewModel.selectPreviousDraft()
        }

        Item { Layout.fillWidth: true }

        Controls.DangerButton {
            objectName: "importClearButton"
            text: qsTr("Clear")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.importViewModel.canClearImport
            enabled: root.importViewModel.canClearImport
            onClicked: root.importViewModel.clearImport()
        }

        Controls.DangerButton {
            objectName: "importCancelButton"
            text: qsTr("Cancel")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.importViewModel.canCancel
            enabled: root.importViewModel.canCancel
            onClicked: root.importViewModel.cancelCurrentImport()
        }

        Controls.DangerButton {
            objectName: "importCancelAllButton"
            text: qsTr("Cancel all")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.importViewModel.canCancel
                     && root.importViewModel.queuedCount > 0
            enabled: root.importViewModel.canCancel
                     && root.importViewModel.queuedCount > 0
            onClicked: root.importViewModel.cancelAllImports()
        }

        Controls.SuccessButton {
            objectName: "importPauseButton"
            text: root.importViewModel.pauseText
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.importViewModel.canPause
            enabled: root.importViewModel.canPause
            onClicked: root.importViewModel.importPaused
                       ? root.importViewModel.resumeImport()
                       : root.importViewModel.pauseImport()
        }

        Controls.SuccessButton {
            objectName: "importStartButton"
            text: qsTr("Start")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: !root.importViewModel.importRunning
            enabled: root.importViewModel.canStart
            onClicked: root.importViewModel.startImport()
        }

        Item { Layout.fillWidth: true }

        Controls.NextPageButton {
            objectName: "importNextDraftButton"
            enabled: root.importViewModel.hasDraftNavigation
            onClicked: root.importViewModel.selectNextDraft()
        }
    }
}
