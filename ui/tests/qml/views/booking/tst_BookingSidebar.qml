/**
 * @file ui/tests/qml/views/booking/tst_BookingSidebar.qml
 * @brief Provides QML tests for BookingSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Booking 1.0 as Booking

import "../../common"

TestCase {
    id: testCase
    name: "BookingSidebarTests"
    when: windowShown
    width: 320
    height: 240

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var bookingViewModel: QtObject {
        property string selectedStatementId: ""
        property string selectedTransactionId: ""
        property var statementRows: [
            {
                id: "statement-1",
                name: "January",
                transactions: [
                    { id: "transaction-1", name: "Rent", bookingDate: "2026-01-01" },
                    { id: "transaction-2", name: "Power", bookingDate: "2026-01-02" }
                ]
            },
            {
                id: "statement-2",
                name: "February",
                transactions: [
                    { id: "transaction-3", name: "Water", bookingDate: "2026-02-01" }
                ]
            }
        ]
        function selectStatement(statementId) {
            selectedStatementId = statementId
            selectedTransactionId = ""
        }
        function selectTransaction(statementId, transactionId) {
            selectedStatementId = statementId
            selectedTransactionId = transactionId
        }
    }

    Component {
        id: sidebarComponent
        Booking.BookingSidebar {
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

    function createSidebar() {
        return createTemporaryObject(sidebarComponent, testCase)
    }

    function init() {
        bookingViewModel.selectedStatementId = ""
        bookingViewModel.selectedTransactionId = ""
    }

    function test_BKG_S_001_transactionRowClickSelectsStatementAndTransaction() {
        const sidebar = createSidebar()

        findRequired(sidebar, "bookingTransactionMouse_transaction-2").clicked(null)

        compare(bookingViewModel.selectedStatementId, "statement-1")
        compare(bookingViewModel.selectedTransactionId, "transaction-2")
    }

    function test_BKG_S_002_statementRowClickClearsTransactionSelection() {
        bookingViewModel.selectedStatementId = "statement-1"
        bookingViewModel.selectedTransactionId = "transaction-1"
        const sidebar = createSidebar()

        findRequired(sidebar, "bookingStatementMouse_statement-2").clicked(null)

        compare(bookingViewModel.selectedStatementId, "statement-2")
        compare(bookingViewModel.selectedTransactionId, "")
    }

    function test_BKG_S_003_rowsRefreshWhenStatementRowsChange() {
        const sidebar = createSidebar()
        verify(findRequired(sidebar, "bookingTransactionRow_transaction-2") !== null)

        bookingViewModel.statementRows = [
            {
                id: "statement-1",
                name: "January",
                transactions: [
                    { id: "transaction-1", name: "Rent", bookingDate: "2026-01-01" }
                ]
            }
        ]
        wait(50)

        compare(Lookup.findObject(sidebar, "bookingTransactionRow_transaction-2"), null)
    }
}
