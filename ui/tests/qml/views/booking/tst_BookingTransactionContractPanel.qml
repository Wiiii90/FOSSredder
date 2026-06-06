/**
 * @file ui/tests/qml/views/booking/tst_BookingTransactionContractPanel.qml
 * @brief Provides QML tests for BookingTransactionContractPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Booking 1.0 as Booking

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "BookingTransactionContractPanelTests"
    when: windowShown
    width: 360
    height: 120

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var bookingViewModel: QtObject {
        property var contractDisplayRows: [{ id: "", display: "No contract" }, { id: "contract-1", display: "Lease" }]
        property int selectedContractIndex: 0
        property int lastContractIndex: -1
        function selectContractIndex(index) { lastContractIndex = index; selectedContractIndex = index }
    }

    Component {
        id: panelComponent
        Booking.BookingTransactionContractPanel { theme: testCase.theme; bookingViewModel: testCase.bookingViewModel }
    }

    function findRequired(root, objectName) {
        const found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function test_BKG_TC_001_contractSelectionCallsBookingState() {
        const panel = createTemporaryObject(panelComponent, testCase)

        findRequired(panel, "bookingTransactionContractComboBox").activated(1)

        compare(bookingViewModel.lastContractIndex, 1)
    }
}
