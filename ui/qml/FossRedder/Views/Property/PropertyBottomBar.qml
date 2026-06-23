/**
 * @file ui/qml/FossRedder/Views/Property/PropertyBottomBar.qml
 * @brief Provides the PropertyBottomBar component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls

pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var propertyViewModel
    required property var propertyRows

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: parent
        theme: root.theme

        Controls.PrevButton {
            objectName: "propertyPreviousButton"
            enabled: root.propertyRows.length > 0
            onClicked: if (root.propertyViewModel) root.propertyViewModel.previous()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.DangerButton {
            objectName: "propertyClearButton"
            visible: root.propertyViewModel ? !root.propertyViewModel.isEdit : false
            text: qsTr("Clear")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.propertyViewModel) root.propertyViewModel.clear()
        }

        Controls.SuccessButton {
            objectName: "propertyCreateButton"
            visible: root.propertyViewModel ? !root.propertyViewModel.isEdit : false
            text: qsTr("Create")
            enabled: root.propertyViewModel ? root.propertyViewModel.canSubmit : false
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.propertyViewModel) root.propertyViewModel.submit()
        }

        Controls.DangerButton {
            objectName: "propertyDeleteButton"
            visible: root.propertyViewModel ? root.propertyViewModel.isEdit : false
            text: qsTr("Delete")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.propertyViewModel) root.propertyViewModel.deleteCurrent()
        }

        Controls.SuccessButton {
            objectName: "propertyUpdateButton"
            visible: root.propertyViewModel ? root.propertyViewModel.isEdit : false
            text: qsTr("Update")
            enabled: root.propertyViewModel ? (root.propertyViewModel.hasChanges && root.propertyViewModel.canSubmit) : false
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.propertyViewModel) root.propertyViewModel.submit()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.NextButton {
            objectName: "propertyNextButton"
            enabled: root.propertyRows.length > 0
            onClicked: if (root.propertyViewModel) root.propertyViewModel.next()
        }
    }
}
