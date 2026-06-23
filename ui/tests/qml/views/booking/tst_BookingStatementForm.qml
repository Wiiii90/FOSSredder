/**
 * @file ui/tests/qml/views/booking/tst_BookingStatementForm.qml
 * @brief Provides QML wiring tests for BookingStatementForm.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Booking 1.0 as Booking

import "../../common"

TestCase {
    id: testCase
    name: "BookingStatementFormTests"
    when: windowShown
    width: 720
    height: 420

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var bookingViewModel: QtObject {
        property string statementName: "Statement"
        property string transactionInfoText: "Transaction 1 / 1"
        property bool canDeleteTransaction: true
        property bool canAddTransaction: true
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
        property var propertyRows: []
        property var selectedPropertyIds: []
        property int selectedActorIndex: 0
        property int selectedContractIndex: 0
        property int addCalls: 0
        property int deleteCalls: 0
        function addTransactionAfterCurrent() { addCalls += 1 }
        function deleteCurrentTransaction() { deleteCalls += 1 }
        function selectActorIndex(index) {}
        function selectContractIndex(index) {}
        function isPropertySelected(propertyId) { return selectedPropertyIds.indexOf(propertyId) !== -1 }
        function setPropertySelected(propertyId, selected) {}
    }

    Component {
        id: formComponent

        Booking.BookingStatementForm {
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

    function init() {
        bookingViewModel.statementName = "Statement"
        bookingViewModel.transactionInfoText = "Transaction 1 / 1"
        bookingViewModel.canAddTransaction = true
        bookingViewModel.canDeleteTransaction = true
        bookingViewModel.transactionName = "Rent"
        bookingViewModel.addCalls = 0
        bookingViewModel.deleteCalls = 0
    }

    function test_BKG_SF_001_statementNameWritesDirectlyToBookingState() {
        const form = createForm()
        const nameField = findRequired(form, "bookingStatementNameField")

        nameField.text = "Statement Updated"
        nameField.textEdited()

        compare(bookingViewModel.statementName, "Statement Updated")
    }

    function test_BKG_SF_002_transactionCommandButtonsCallBookingState() {
        const form = createForm()

        findRequired(form, "bookingStatementAddTransactionButton").clicked()
        findRequired(form, "bookingStatementRemoveTransactionButton").clicked()

        compare(bookingViewModel.addCalls, 1)
        compare(bookingViewModel.deleteCalls, 1)
    }

    function test_BKG_SF_003_transactionViewIsComposedInsideStatementPanel() {
        const form = createForm()

        verify(findRequired(form, "bookingTransactionViewRoot") !== null)
        compare(findRequired(form, "bookingTransactionNameField").text, "Rent")
    }
}
