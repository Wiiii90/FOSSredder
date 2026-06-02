/**
 * @file ui/tests/qml/views/booking/tst_BookingTransactionView.qml
 * @brief Provides QML composition tests for BookingTransactionView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Booking 1.0 as Booking

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "BookingTransactionViewTests"
    when: windowShown
    width: 720
    height: 520

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacingMedium: 10
        property int radius: 3
        property int controlHeight: 40
        property int borderWidthThin: 1
        property string fontFamily: "Arial"
        property int fontSize: 10
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color borderSoft: "#dddddd"
        property color textPrimary: "#111111"
        property color button: "#eeeeee"
        property color buttonHover: "#dddddd"
        property color buttonPressed: "#cccccc"
        property color buttonText: "#111111"
    }

    property var bookingViewModel: QtObject {
        property string transactionName: "Rent"
        property string transactionBookingDate: ""
        property string transactionValuta: ""
        property string transactionAmountText: ""
        property int transactionStatusIndex: 0
        property var transactionStatusOptions: [
            { label: "Neutral", value: 0 },
            { label: "Unverified", value: 1 },
            { label: "Verified", value: 2 },
            { label: "Completed", value: 3 }
        ]
        property bool transactionAllocatable: false
        property var actorDisplayRows: [{ id: "", display: "No actor" }]
        property var contractDisplayRows: [{ id: "", display: "No contract" }]
        property var propertyRows: [{ id: "property-1", display: "Flat 1" }]
        property var selectedPropertyIds: []
        property int selectedActorIndex: 0
        property int selectedContractIndex: 0
        function selectActorIndex(index) {}
        function selectContractIndex(index) {}
        function isPropertySelected(propertyId) { return selectedPropertyIds.indexOf(propertyId) !== -1 }
        function setPropertySelected(propertyId, selected) {}
    }

    Component {
        id: viewComponent

        Booking.BookingTransactionView {
            width: testCase.width
            theme: testCase.theme
            bookingViewModel: testCase.bookingViewModel
        }
    }

    function findRequired(root, objectName) {
        const found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function test_BKG_TV_001_composesTransactionFormAndPanels() {
        const view = createTemporaryObject(viewComponent, testCase)

        verify(findRequired(view, "bookingTransactionNameField") !== null)
        verify(findRequired(view, "bookingTransactionActorComboBox") !== null)
        verify(findRequired(view, "bookingTransactionContractComboBox") !== null)
        verify(findRequired(view, "bookingTransactionPropertyCheckBox") !== null)
        verify(findRequired(view, "bookingTransactionAllocatableToggle") !== null)
    }
}
