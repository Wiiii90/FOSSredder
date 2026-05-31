/**
 * @file ui/tests/qml/views/booking/tst_BookingBottomBar.qml
 * @brief Provides QML tests for BookingBottomBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Booking 1.0 as Booking

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "BookingBottomBarTests"
    when: windowShown
    width: 720
    height: 96

    property var theme: QtObject {
        property int spacingSmall: 6
        property int viewActionButtonWidth: 96
        property int margins: 4
        property int radius: 3
        property int borderWidthThin: 1
        property color borderSoft: "#cccccc"
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
    }

    property var bookingState: QtObject {
        property bool isCreateMode: true
        property bool hasStatements: true
        property bool hasMultipleTransactions: true
        property bool canCreate: true
        property bool canUpdate: true
        property int previousStatementCalls: 0
        property int nextStatementCalls: 0
        property int previousTransactionCalls: 0
        property int nextTransactionCalls: 0
        property int clearCalls: 0
        property int createCalls: 0
        property int deleteCalls: 0
        property int updateCalls: 0
        function previousStatement() { previousStatementCalls += 1 }
        function nextStatement() { nextStatementCalls += 1 }
        function previousTransaction() { previousTransactionCalls += 1 }
        function nextTransaction() { nextTransactionCalls += 1 }
        function resetCreateState() { clearCalls += 1 }
        function submit() { createCalls += 1 }
        function deleteCurrentStatement() { deleteCalls += 1 }
        function updateCurrent() { updateCalls += 1 }
    }

    Component {
        id: bottomBarComponent

        Booking.BookingBottomBar {
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

    function createBar() {
        return createTemporaryObject(bottomBarComponent, testCase)
    }

    function init() {
        bookingState.isCreateMode = true
        bookingState.hasStatements = true
        bookingState.hasMultipleTransactions = true
        bookingState.canCreate = true
        bookingState.canUpdate = true
        bookingState.previousStatementCalls = 0
        bookingState.nextStatementCalls = 0
        bookingState.previousTransactionCalls = 0
        bookingState.nextTransactionCalls = 0
        bookingState.clearCalls = 0
        bookingState.createCalls = 0
        bookingState.deleteCalls = 0
        bookingState.updateCalls = 0
    }

    function test_BKG_BB_001_navigationButtonsCallStateNavigation() {
        const bar = createBar()

        findRequired(bar, "bookingPreviousStatementButton").clicked()
        findRequired(bar, "bookingNextStatementButton").clicked()
        findRequired(bar, "bookingPreviousTransactionButton").clicked()
        findRequired(bar, "bookingNextTransactionButton").clicked()

        compare(bookingState.previousStatementCalls, 1)
        compare(bookingState.nextStatementCalls, 1)
        compare(bookingState.previousTransactionCalls, 1)
        compare(bookingState.nextTransactionCalls, 1)
    }

    function test_BKG_BB_002_createModeButtonsCallStateCommands() {
        const bar = createBar()

        findRequired(bar, "bookingClearButton").clicked()
        findRequired(bar, "bookingCreateButton").clicked()

        compare(bookingState.clearCalls, 1)
        compare(bookingState.createCalls, 1)
    }

    function test_BKG_BB_003_editModeButtonsCallStateCommands() {
        bookingState.isCreateMode = false
        const bar = createBar()

        findRequired(bar, "bookingDeleteButton").clicked()
        findRequired(bar, "bookingUpdateButton").clicked()

        compare(bookingState.deleteCalls, 1)
        compare(bookingState.updateCalls, 1)
    }
}
