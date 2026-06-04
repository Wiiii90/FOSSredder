/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualBottomBar.qml
 * @brief Provides the Annual view action bar.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var annualViewModel

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: root
        theme: root.theme

        Controls.PrevButton {
            objectName: "annualPreviousButton"
            enabled: root.annualViewModel.hasRows
            onClicked: root.annualViewModel.navigate(-1)
        }

        Item { Layout.fillWidth: true }

        Controls.SecondaryButton {
            objectName: "annualToggleContentButton"
            text: "⇆"
            Layout.preferredWidth: root.theme.viewNavigationButtonWidth
            onClicked: root.annualViewModel.toggleContent()
        }

        Controls.DangerButton {
            objectName: "annualClearButton"
            visible: !root.annualViewModel.isEdit
            text: qsTr("Clear")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.annualViewModel.resetCreateState()
        }

        Controls.SuccessButton {
            objectName: "annualCreateButton"
            visible: !root.annualViewModel.isEdit
            text: qsTr("Create")
            enabled: root.annualViewModel.canSubmit
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.annualViewModel.submitCreate()
        }

        Controls.DangerButton {
            objectName: "annualDeleteButton"
            visible: root.annualViewModel.isEdit
            text: qsTr("Delete")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.annualViewModel.deleteCurrent()
        }

        Controls.SuccessButton {
            objectName: "annualUpdateButton"
            visible: root.annualViewModel.isEdit
            text: qsTr("Update")
            enabled: root.annualViewModel.canSubmit && root.annualViewModel.hasChanges
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.annualViewModel.submitUpdate()
        }

        Item { Layout.fillWidth: true }

        Controls.NextButton {
            objectName: "annualNextButton"
            enabled: root.annualViewModel.hasRows
            onClicked: root.annualViewModel.navigate(1)
        }
    }
}
