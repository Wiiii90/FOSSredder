/**
 * @file ui/qml/FossRedder/Views/Contract/ContractBottomBar.qml
 * @brief Provides the ContractBottomBar component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls

pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var contractState
    required property var contractRows

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: parent
        theme: root.theme

        Controls.PrevButton {
            objectName: "contractPreviousButton"
            enabled: root.contractRows.length > 0
            onClicked: if (root.contractState) root.contractState.previous()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.DangerButton {
            objectName: "contractClearButton"
            visible: root.contractState ? !root.contractState.isEdit : false
            text: qsTr("Clear")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.contractState) root.contractState.clear()
        }

        Controls.SuccessButton {
            objectName: "contractCreateButton"
            visible: root.contractState ? !root.contractState.isEdit : false
            text: qsTr("Create")
            enabled: root.contractState ? root.contractState.canSubmit : false
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.contractState) root.contractState.submit()
        }

        Controls.DangerButton {
            objectName: "contractDeleteButton"
            visible: root.contractState ? root.contractState.isEdit : false
            text: qsTr("Delete")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.contractState) root.contractState.deleteCurrent()
        }

        Controls.SecondaryButton {
            objectName: "contractCreateModeButton"
            visible: root.contractState ? root.contractState.isEdit : false
            text: qsTr("New")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.contractState) root.contractState.enterCreateMode()
        }

        Controls.SuccessButton {
            objectName: "contractUpdateButton"
            visible: root.contractState ? root.contractState.isEdit : false
            text: qsTr("Update")
            enabled: root.contractState ? (root.contractState.hasChanges && root.contractState.canSubmit) : false
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.contractState) root.contractState.submit()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.NextButton {
            objectName: "contractNextButton"
            enabled: root.contractRows.length > 0
            onClicked: if (root.contractState) root.contractState.next()
        }
    }
}
