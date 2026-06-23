/**
 * @file ui/qml/FossRedder/Views/Booking/BookingView.qml
 * @brief Provides the BookingView component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views.Booking 1.0 as Booking
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var bookingViewModel
    required property var theme

    ColumnLayout {
        anchors.fill: root
        anchors.margins: root.theme.pageContentMargin
        spacing: root.theme.spacingSmall

        Booking.BookingStatementView {
            id: statementView
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: root.theme
            bookingViewModel: root.bookingViewModel
        }

        Booking.BookingBottomBar {
            Layout.fillWidth: true
            theme: root.theme
            bookingViewModel: root.bookingViewModel
        }
    }
}
