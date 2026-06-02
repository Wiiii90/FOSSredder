/**
 * @file ui/qml/FossRedder/Views/Actor/ActorBottomBar.qml
 * @brief Provides the ActorBottomBar component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls

pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var actorViewModel
    required property var actorRows

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: parent
        theme: root.theme

        Controls.PrevButton {
            objectName: "actorPreviousButton"
            enabled: root.actorRows.length > 0
            onClicked: if (root.actorViewModel) root.actorViewModel.previous()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.DangerButton {
            objectName: "actorClearButton"
            visible: root.actorViewModel ? !root.actorViewModel.isEdit : false
            text: qsTr("Clear")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.actorViewModel) root.actorViewModel.clear()
        }

        Controls.SuccessButton {
            objectName: "actorCreateButton"
            visible: root.actorViewModel ? !root.actorViewModel.isEdit : false
            text: qsTr("Create")
            enabled: root.actorViewModel ? root.actorViewModel.canSubmit : false
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.actorViewModel) root.actorViewModel.submit()
        }

        Controls.DangerButton {
            objectName: "actorDeleteButton"
            visible: root.actorViewModel ? root.actorViewModel.isEdit : false
            text: qsTr("Delete")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.actorViewModel) root.actorViewModel.deleteCurrent()
        }

        Controls.SecondaryButton {
            objectName: "actorCreateModeButton"
            visible: root.actorViewModel ? root.actorViewModel.isEdit : false
            text: qsTr("New")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.actorViewModel) root.actorViewModel.enterCreateMode()
        }

        Controls.SuccessButton {
            objectName: "actorUpdateButton"
            visible: root.actorViewModel ? root.actorViewModel.isEdit : false
            text: qsTr("Update")
            enabled: root.actorViewModel ? (root.actorViewModel.hasChanges && root.actorViewModel.canSubmit) : false
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.actorViewModel) root.actorViewModel.submit()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.NextButton {
            objectName: "actorNextButton"
            enabled: root.actorRows.length > 0
            onClicked: if (root.actorViewModel) root.actorViewModel.next()
        }
    }
}
