/**
 * @file ui/tests/qml/views/booking/tst_BookingTransactionView.qml
 * @brief Provides QML composition tests for BookingTransactionView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Booking 1.0 as Booking

import "../../common"

TestCase {
    id: testCase
    name: "BookingTransactionViewTests"
    when: windowShown
    width: 720
    height: 520

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

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
