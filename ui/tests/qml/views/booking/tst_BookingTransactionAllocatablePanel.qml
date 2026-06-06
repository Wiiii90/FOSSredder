/**
 * @file ui/tests/qml/views/booking/tst_BookingTransactionAllocatablePanel.qml
 * @brief Provides QML tests for BookingTransactionAllocatablePanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Booking 1.0 as Booking

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "BookingTransactionAllocatablePanelTests"
    when: windowShown
    width: 360
    height: 120

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var bookingViewModel: QtObject {
        property bool transactionAllocatable: false
    }

    Component {
        id: panelComponent
        Booking.BookingTransactionAllocatablePanel { theme: testCase.theme; bookingViewModel: testCase.bookingViewModel }
    }

    function findRequired(root, objectName) {
        const found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function test_BKG_TA_002_allocatableToggleWritesBookingState() {
        const panel = createTemporaryObject(panelComponent, testCase)

        findRequired(panel, "bookingTransactionAllocatableToggle").clicked()

        compare(bookingViewModel.transactionAllocatable, true)
    }
}
