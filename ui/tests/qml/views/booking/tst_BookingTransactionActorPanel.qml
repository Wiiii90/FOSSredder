/**
 * @file ui/tests/qml/views/booking/tst_BookingTransactionActorPanel.qml
 * @brief Provides QML tests for BookingTransactionActorPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Booking 1.0 as Booking

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "BookingTransactionActorPanelTests"
    when: windowShown
    width: 360
    height: 120

    property var theme: QtObject {
        property int spacingSmall: 6
        property int radius: 3
        property int borderWidthThin: 1
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
    }

    property var bookingViewModel: QtObject {
        property var actorDisplayRows: [{ id: "", display: "No actor" }, { id: "actor-1", display: "Alice" }]
        property int selectedActorIndex: 0
        property int lastActorIndex: -1
        function selectActorIndex(index) { lastActorIndex = index; selectedActorIndex = index }
    }

    Component {
        id: panelComponent
        Booking.BookingTransactionActorPanel { theme: testCase.theme; bookingViewModel: testCase.bookingViewModel }
    }

    function findRequired(root, objectName) {
        const found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function test_BKG_TA_001_actorSelectionCallsBookingState() {
        const panel = createTemporaryObject(panelComponent, testCase)

        findRequired(panel, "bookingTransactionActorComboBox").activated(1)

        compare(bookingViewModel.lastActorIndex, 1)
    }
}
