/**
 * @file ui/tests/qml/views/booking/tst_BookingView.qml
 * @brief Provides QML composition tests for BookingView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Booking 1.0 as Booking

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "BookingViewTests"
    when: windowShown
    width: 900
    height: 620

    property int submitCalls: 0
    property int updateCalls: 0
    property int clearCalls: 0

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var bookingViewModel: QtObject {
        property bool isCreateMode: true
        property bool hasStatements: true
        property bool hasMultipleTransactions: true
        property bool canCreate: true
        property bool canUpdate: true
        property bool canDeleteTransaction: true
        property bool canAddTransaction: true
        property string statementName: "Statement"
        property string transactionInfoText: "Transaction 1 / 1"
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
        function previousStatement() {}
        function nextStatement() {}
        function previousTransaction() {}
        function nextTransaction() {}
        function addTransactionAfterCurrent() {}
        function deleteCurrentTransaction() {}
        function resetCreateState() { testCase.clearCalls += 1 }
        function submit() { testCase.submitCalls += 1 }
        function updateCurrent() { testCase.updateCalls += 1 }
        function deleteCurrentStatement() {}
        function selectActorIndex(index) {}
        function selectContractIndex(index) {}
        function isPropertySelected(propertyId) { return selectedPropertyIds.indexOf(propertyId) !== -1 }
        function setPropertySelected(propertyId, selected) {}
    }

    Component {
        id: viewComponent

        Booking.BookingView {
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

    function createView() {
        return createTemporaryObject(viewComponent, testCase)
    }

    function init() {
        submitCalls = 0
        updateCalls = 0
        clearCalls = 0
        bookingViewModel.isCreateMode = true
    }

    function test_BKG_V_001_createModeContainerMountsAndCallsStateActions() {
        const view = createView()

        findRequired(view, "bookingClearButton").clicked()
        findRequired(view, "bookingCreateButton").clicked()

        compare(clearCalls, 1)
        compare(submitCalls, 1)
    }

    function test_BKG_V_002_editModeContainerMountsAndCallsUpdate() {
        bookingViewModel.isCreateMode = false
        const view = createView()

        findRequired(view, "bookingUpdateButton").clicked()

        compare(updateCalls, 1)
    }
}
