/**
 * @file ui/qml/FossRedder/Views/Booking/BookingStatementView.qml
 * @brief Provides the BookingStatementView component.
 */

import QtQuick 2.15
import FossRedder.Views.Booking 1.0 as Booking
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var bookingViewModel

    Booking.BookingStatementForm {
        anchors.fill: root
        theme: root.theme
        bookingViewModel: root.bookingViewModel
    }
}
