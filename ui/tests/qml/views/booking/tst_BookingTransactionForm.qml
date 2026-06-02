/**
 * @file ui/tests/qml/views/booking/tst_BookingTransactionForm.qml
 * @brief Provides QML tests for BookingTransactionForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Booking 1.0 as Booking

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "BookingTransactionFormTests"
    when: windowShown
    width: 720
    height: 220

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacingMedium: 10
        property int controlHeight: 40
        property string fontFamily: "Arial"
        property int fontSize: 10
    }

    property var bookingViewModel: QtObject {
        property string transactionName: "Rent"
        property string transactionBookingDate: "2026-01-05"
        property string transactionValuta: "2026-01-06"
        property string transactionAmountText: "12.50"
        property int transactionStatusIndex: 0
        property var transactionStatusOptions: [
            { label: "Neutral", value: 0 },
            { label: "Unverified", value: 1 },
            { label: "Verified", value: 2 },
            { label: "Completed", value: 3 }
        ]
    }

    Component {
        id: formComponent

        Booking.BookingTransactionForm {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            bookingViewModel: testCase.bookingViewModel
        }
    }

    function findRequired(root, objectName) {
        const found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function createForm() {
        return createTemporaryObject(formComponent, testCase)
    }

    function test_BKG_TF_001_transactionFieldsWriteBookingState() {
        const form = createForm()

        const nameField = findRequired(form, "bookingTransactionNameField")
        nameField.text = "Rent Updated"
        nameField.textEdited()

        const bookingDateField = findRequired(form, "bookingTransactionBookingDateField")
        bookingDateField.text = "2026-02-01"
        bookingDateField.textEdited()

        const valutaField = findRequired(form, "bookingTransactionValutaField")
        valutaField.text = "2026-02-02"
        valutaField.textEdited()

        const amountField = findRequired(form, "bookingTransactionAmountField")
        amountField.text = "99.95"
        amountField.textEdited()

        findRequired(form, "bookingTransactionStatusComboBox").activated(2)

        compare(bookingViewModel.transactionName, "Rent Updated")
        compare(bookingViewModel.transactionBookingDate, "2026-02-01")
        compare(bookingViewModel.transactionValuta, "2026-02-02")
        compare(bookingViewModel.transactionAmountText, "99.95")
        compare(bookingViewModel.transactionStatusIndex, 2)
    }
}
