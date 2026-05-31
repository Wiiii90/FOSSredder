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
    required property var propertyState
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
            onClicked: if (root.propertyState) root.propertyState.previous()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.DangerButton {
            objectName: "propertyClearButton"
            visible: root.propertyState ? !root.propertyState.isEdit : false
            text: qsTr("Clear")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.propertyState) root.propertyState.clear()
        }

        Controls.SuccessButton {
            objectName: "propertyCreateButton"
            visible: root.propertyState ? !root.propertyState.isEdit : false
            text: qsTr("Create")
            enabled: root.propertyState ? root.propertyState.canSubmit : false
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.propertyState) root.propertyState.submit()
        }

        Controls.DangerButton {
            objectName: "propertyDeleteButton"
            visible: root.propertyState ? root.propertyState.isEdit : false
            text: qsTr("Delete")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.propertyState) root.propertyState.deleteCurrent()
        }

        Controls.SecondaryButton {
            objectName: "propertyCreateModeButton"
            visible: root.propertyState ? root.propertyState.isEdit : false
            text: qsTr("New")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.propertyState) root.propertyState.enterCreateMode()
        }

        Controls.SuccessButton {
            objectName: "propertyUpdateButton"
            visible: root.propertyState ? root.propertyState.isEdit : false
            text: qsTr("Update")
            enabled: root.propertyState ? (root.propertyState.hasChanges && root.propertyState.canSubmit) : false
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: if (root.propertyState) root.propertyState.submit()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.NextButton {
            objectName: "propertyNextButton"
            enabled: root.propertyRows.length > 0
            onClicked: if (root.propertyState) root.propertyState.next()
        }
    }
}
