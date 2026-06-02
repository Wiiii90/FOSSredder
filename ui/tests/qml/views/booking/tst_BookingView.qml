/**
 * @file ui/tests/qml/views/booking/tst_BookingView.qml
 * @brief Provides QML composition tests for BookingView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
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

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int spacing: 8
        property int spacingSmall: 6
        property int spacingMedium: 10
        property int spacingLarge: 16
        property int margins: 4
        property int radius: 3
        property int formLabelWidth: 110
        property int controlHeight: 40
        property int viewCompactActionButtonSize: 26
        property int viewActionButtonWidth: 96
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
        property color danger: "#cc3333"
        property color dangerHover: "#bb2222"
        property color dangerPressed: "#aa1111"
        property color dangerText: "#ffffff"
        property color success: "#228833"
        property color successHover: "#227733"
        property color successPressed: "#116622"
        property color successText: "#ffffff"
        property color selectionHighlight: "#aaccee"
    }

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
