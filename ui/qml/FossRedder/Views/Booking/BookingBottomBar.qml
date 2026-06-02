/**
 * @file ui/qml/FossRedder/Views/Booking/BookingBottomBar.qml
 * @brief Provides the Booking bottom action bar.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls

pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var bookingViewModel

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: root
        theme: root.theme

        Controls.PrevPageButton {
            objectName: "bookingPreviousStatementButton"
            enabled: root.bookingViewModel.hasStatements
            onClicked: root.bookingViewModel.previousStatement()
        }

        Controls.PrevButton {
            objectName: "bookingPreviousTransactionButton"
            enabled: root.bookingViewModel.hasMultipleTransactions
            onClicked: root.bookingViewModel.previousTransaction()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.DangerButton {
            objectName: "bookingClearButton"
            visible: root.bookingViewModel.isCreateMode
            text: qsTr("Clear")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.bookingViewModel.resetCreateState()
        }

        Controls.SuccessButton {
            objectName: "bookingCreateButton"
            visible: root.bookingViewModel.isCreateMode
            text: qsTr("Create")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            enabled: root.bookingViewModel.canCreate
            onClicked: root.bookingViewModel.submit()
        }

        Controls.DangerButton {
            objectName: "bookingDeleteButton"
            visible: !root.bookingViewModel.isCreateMode
            text: qsTr("Delete")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.bookingViewModel.deleteCurrentStatement()
        }

        Controls.SuccessButton {
            objectName: "bookingUpdateButton"
            visible: !root.bookingViewModel.isCreateMode
            text: qsTr("Update")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            enabled: root.bookingViewModel.canUpdate
            onClicked: root.bookingViewModel.updateCurrent()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.NextButton {
            objectName: "bookingNextTransactionButton"
            enabled: root.bookingViewModel.hasMultipleTransactions
            onClicked: root.bookingViewModel.nextTransaction()
        }

        Controls.NextPageButton {
            objectName: "bookingNextStatementButton"
            enabled: root.bookingViewModel.hasStatements
            onClicked: root.bookingViewModel.nextStatement()
        }
    }
}
