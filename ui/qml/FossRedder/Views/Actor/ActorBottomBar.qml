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
    required property var actorState
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
            onClicked: if (root.actorState) root.actorState.previous()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.DangerButton {
            objectName: "actorClearButton"
            visible: root.actorState ? !root.actorState.isEdit : false
            text: qsTr("Clear")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.actorState) root.actorState.clear()
        }

        Controls.SuccessButton {
            objectName: "actorCreateButton"
            visible: root.actorState ? !root.actorState.isEdit : false
            text: qsTr("Create")
            enabled: root.actorState ? root.actorState.canSubmit : false
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.actorState) root.actorState.submit()
        }

        Controls.DangerButton {
            objectName: "actorDeleteButton"
            visible: root.actorState ? root.actorState.isEdit : false
            text: qsTr("Delete")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.actorState) root.actorState.deleteCurrent()
        }

        Controls.SecondaryButton {
            objectName: "actorCreateModeButton"
            visible: root.actorState ? root.actorState.isEdit : false
            text: qsTr("New")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.actorState) root.actorState.enterCreateMode()
        }

        Controls.SuccessButton {
            objectName: "actorUpdateButton"
            visible: root.actorState ? root.actorState.isEdit : false
            text: qsTr("Update")
            enabled: root.actorState ? (root.actorState.hasChanges && root.actorState.canSubmit) : false
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.actorState) root.actorState.submit()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.NextButton {
            objectName: "actorNextButton"
            enabled: root.actorRows.length > 0
            onClicked: if (root.actorState) root.actorState.next()
        }
    }
}
