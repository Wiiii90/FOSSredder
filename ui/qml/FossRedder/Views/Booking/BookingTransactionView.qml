/**
 * @file ui/qml/FossRedder/Views/Booking/BookingTransactionView.qml
 * @brief Provides the BookingTransactionView component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views.Booking 1.0 as Booking
pragma ComponentBehavior: Bound

Item {
    id: root
    objectName: "bookingTransactionViewRoot"
    required property var theme
    required property var bookingViewModel

    implicitHeight: txLayout.implicitHeight

    ColumnLayout {
        id: txLayout
        width: root.width
        spacing: root.theme.spacingSmall

        Booking.BookingTransactionForm {
            Layout.fillWidth: true
            theme: root.theme
            bookingViewModel: root.bookingViewModel
        }

        Booking.BookingTransactionActorPanel {
            theme: root.theme
            bookingViewModel: root.bookingViewModel
        }

        Booking.BookingTransactionContractPanel {
            theme: root.theme
            bookingViewModel: root.bookingViewModel
        }

        Booking.BookingTransactionPropertyPanel {
            theme: root.theme
            bookingViewModel: root.bookingViewModel
        }

        Booking.BookingTransactionAllocatablePanel {
            theme: root.theme
            bookingViewModel: root.bookingViewModel
        }
    }
}
