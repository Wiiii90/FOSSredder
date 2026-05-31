/**
 * @file ui/tests/qml/views/booking/tst_BookingStatementView.qml
 * @brief Provides QML wiring tests for BookingStatementView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Booking 1.0 as Booking

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "BookingStatementViewTests"
    when: windowShown
    width: 760
    height: 420

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacingMedium: 10
        property int spacingLarge: 16
        property int margins: 4
        property int radius: 3
        property int formLabelWidth: 110
        property int controlHeight: 40
        property int viewCompactActionButtonSize: 26
        property int borderWidthThin: 1
        property string fontFamily: "Arial"
        property int fontSize: 10
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color borderSoft: "#dddddd"
        property color textPrimary: "#111111"
        property color textMuted: "#777777"
        property color button: "#eeeeee"
        property color buttonHover: "#dddddd"
        property color buttonPressed: "#cccccc"
        property color buttonText: "#111111"
    }

    property var bookingState: QtObject {
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
        id: viewComponent

        Booking.BookingStatementView {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            bookingState: testCase.bookingState
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
        bookingState.statementName = "Statement"
        bookingState.transactionName = "Rent"
        bookingState.addCalls = 0
        bookingState.deleteCalls = 0
    }

    function test_BKG_SV_001_statementNameFieldWritesToBookingState() {
        const view = createView()
        const field = findRequired(view, "bookingStatementNameField")

        field.text = "Updated Statement"
        field.textEdited()

        compare(bookingState.statementName, "Updated Statement")
    }

    function test_BKG_SV_002_transactionButtonsCallBookingState() {
        const view = createView()

        findRequired(view, "bookingStatementAddTransactionButton").clicked()
        findRequired(view, "bookingStatementRemoveTransactionButton").clicked()

        compare(bookingState.addCalls, 1)
        compare(bookingState.deleteCalls, 1)
    }
}
