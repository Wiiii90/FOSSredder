/**
 * @file ui/tests/qml/views/import/tst_TransactionDraftForm.qml
 * @brief Provides QML tests for TransactionDraftForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "TransactionDraftFormTests"
    when: windowShown
    width: 900
    height: 320

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacingMedium: 8
        property int controlHeight: 32
    }

    property var transactionState: QtObject {
        property string nameText: "Tx 1"
        property string bookingDateText: "2026-05-16"
        property string valutaText: "2026-05-17"
        property string amountText: "12.50"
        property var statusOptions: [{ label: "Open", value: 0 }, { label: "Checked", value: 1 }]
        property int statusIndex: 0
        property int commitNameCalls: 0
        property int commitBookingDateCalls: 0
        property int commitValutaCalls: 0
        property int commitAmountCalls: 0
        function commitNameText() { commitNameCalls += 1 }
        function commitBookingDateText() { commitBookingDateCalls += 1 }
        function commitValutaText() { commitValutaCalls += 1 }
        function commitAmountText() { commitAmountCalls += 1 }
        function setStatusByIndex(index) { statusIndex = index }
    }

    Component {
        id: formComponent
        Import.TransactionDraftForm {
            width: testCase.width
            theme: testCase.theme
            transactionState: testCase.transactionState
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createForm() {
        return createTemporaryObject(formComponent, testCase)
    }

    function init() {
        transactionState.nameText = "Tx 1"
        transactionState.bookingDateText = "2026-05-16"
        transactionState.valutaText = "2026-05-17"
        transactionState.amountText = "12.50"
        transactionState.statusIndex = 0
        transactionState.commitNameCalls = 0
        transactionState.commitBookingDateCalls = 0
        transactionState.commitValutaCalls = 0
        transactionState.commitAmountCalls = 0
    }

    function test_IMP_D_010_amountTextCommitsOnlyOnEditingFinished() {
        const form = createForm()
        const amountField = findRequired(form, "transactionDraftAmountField")

        amountField.text = "99.99"
        amountField.textEdited()

        compare(transactionState.amountText, "99.99")
        compare(transactionState.commitAmountCalls, 0)

        amountField.editingFinished()
        compare(transactionState.commitAmountCalls, 1)
    }

    function test_IMP_D_011_amountFieldFollowsCommittedStateChanges() {
        const form = createForm()
        const amountField = findRequired(form, "transactionDraftAmountField")

        compare(amountField.text, "12.50")
        transactionState.amountText = "42.00"
        wait(0)

        compare(amountField.text, "42.00")
    }

    function test_IMP_D_016_basicFieldsDelegateToTransactionState() {
        const form = createForm()

        const nameField = findRequired(form, "transactionDraftNameField")
        nameField.text = "Updated"
        nameField.textEdited()
        nameField.editingFinished()

        findRequired(form, "transactionDraftStatusCombo").activated(1)

        compare(transactionState.nameText, "Updated")
        compare(transactionState.commitNameCalls, 1)
        compare(transactionState.statusIndex, 1)
    }
}
