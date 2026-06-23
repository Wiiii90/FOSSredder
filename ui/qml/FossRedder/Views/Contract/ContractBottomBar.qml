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
    required property var contractViewModel
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
            onClicked: if (root.contractViewModel) root.contractViewModel.previous()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.DangerButton {
            objectName: "contractClearButton"
            visible: root.contractViewModel ? !root.contractViewModel.isEdit : false
            text: qsTr("Clear")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.contractViewModel) root.contractViewModel.clear()
        }

        Controls.SuccessButton {
            objectName: "contractCreateButton"
            visible: root.contractViewModel ? !root.contractViewModel.isEdit : false
            text: qsTr("Create")
            enabled: root.contractViewModel ? root.contractViewModel.canSubmit : false
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.contractViewModel) root.contractViewModel.submit()
        }

        Controls.DangerButton {
            objectName: "contractDeleteButton"
            visible: root.contractViewModel ? root.contractViewModel.isEdit : false
            text: qsTr("Delete")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.contractViewModel) root.contractViewModel.deleteCurrent()
        }

        Controls.SuccessButton {
            objectName: "contractUpdateButton"
            visible: root.contractViewModel ? root.contractViewModel.isEdit : false
            text: qsTr("Update")
            enabled: root.contractViewModel ? (root.contractViewModel.hasChanges && root.contractViewModel.canSubmit) : false
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.contractViewModel) root.contractViewModel.submit()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.NextButton {
            objectName: "contractNextButton"
            enabled: root.contractRows.length > 0
            onClicked: if (root.contractViewModel) root.contractViewModel.next()
        }
    }
}
