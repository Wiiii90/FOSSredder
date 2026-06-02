/**
 * @file ui/tests/qml/views/booking/tst_BookingTransactionAllocatablePanel.qml
 * @brief Provides QML tests for BookingTransactionAllocatablePanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Booking 1.0 as Booking

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "BookingTransactionAllocatablePanelTests"
    when: windowShown
    width: 360
    height: 120

    property var theme: QtObject {
        property int spacingSmall: 6
        property int controlHeight: 40
        property int radius: 3
        property int borderWidthThin: 1
        property string fontFamily: "Arial"
        property int fontSize: 10
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color textPrimary: "#111111"
    }

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

        findRequired(panel, "bookingTransactionAllocatableToggle").clicked(null)

        compare(bookingViewModel.transactionAllocatable, true)
    }
}
