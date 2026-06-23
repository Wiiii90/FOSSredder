/**
 * @file ui/tests/qml/views/booking/tst_BookingBottomBar.qml
 * @brief Provides QML tests for BookingBottomBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Booking 1.0 as Booking

import "../../common"

TestCase {
    id: testCase
    name: "BookingBottomBarTests"
    when: windowShown
    width: 720
    height: 96

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
            bookingViewModel: testCase.bookingViewModel
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
        bookingViewModel.isCreateMode = true
        bookingViewModel.hasStatements = true
        bookingViewModel.hasMultipleTransactions = true
        bookingViewModel.canCreate = true
        bookingViewModel.canUpdate = true
        bookingViewModel.previousStatementCalls = 0
        bookingViewModel.nextStatementCalls = 0
        bookingViewModel.previousTransactionCalls = 0
        bookingViewModel.nextTransactionCalls = 0
        bookingViewModel.clearCalls = 0
        bookingViewModel.createCalls = 0
        bookingViewModel.deleteCalls = 0
        bookingViewModel.updateCalls = 0
    }

    function test_BKG_BB_001_navigationButtonsCallStateNavigation() {
        const bar = createBar()

        findRequired(bar, "bookingPreviousStatementButton").clicked()
        findRequired(bar, "bookingNextStatementButton").clicked()
        findRequired(bar, "bookingPreviousTransactionButton").clicked()
        findRequired(bar, "bookingNextTransactionButton").clicked()

        compare(bookingViewModel.previousStatementCalls, 1)
        compare(bookingViewModel.nextStatementCalls, 1)
        compare(bookingViewModel.previousTransactionCalls, 1)
        compare(bookingViewModel.nextTransactionCalls, 1)
    }

    function test_BKG_BB_002_createModeButtonsCallStateCommands() {
        const bar = createBar()

        findRequired(bar, "bookingClearButton").clicked()
        findRequired(bar, "bookingCreateButton").clicked()

        compare(bookingViewModel.clearCalls, 1)
        compare(bookingViewModel.createCalls, 1)
    }

    function test_BKG_BB_003_editModeButtonsCallStateCommands() {
        bookingViewModel.isCreateMode = false
        const bar = createBar()

        findRequired(bar, "bookingDeleteButton").clicked()
        findRequired(bar, "bookingUpdateButton").clicked()

        compare(bookingViewModel.deleteCalls, 1)
        compare(bookingViewModel.updateCalls, 1)
    }
}
