/**
 * @file ui/tests/qml/views/import/tst_StatementDraftForm.qml
 * @brief Provides QML tests for StatementDraftForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "StatementDraftFormTests"
    when: windowShown
    width: 900
    height: 180

    property var theme: QtObject {
        property int spacingSmall: 6
        property int formLabelWidth: 120
    }

    property var statementViewModel: QtObject {
        property bool hasDraft: true
        property string statementName: "Draft Name"
        property string transactionInfoText: "Transaction 1 of 2"
        property bool canDeleteTransaction: true
        property int addTransactionCalls: 0
        property int deleteTransactionCalls: 0
        function addTransactionAfterCurrent() { addTransactionCalls += 1 }
        function deleteCurrentTransaction() { deleteTransactionCalls += 1 }
    }

    Component {
        id: formComponent
        Import.StatementDraftForm {
            width: testCase.width
            theme: testCase.theme
            statementViewModel: testCase.statementViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createForm() {
        return createTemporaryObject(formComponent, testCase)
    }

    function init() {
        statementViewModel.hasDraft = true
        statementViewModel.statementName = "Draft Name"
        statementViewModel.canDeleteTransaction = true
        statementViewModel.addTransactionCalls = 0
        statementViewModel.deleteTransactionCalls = 0
    }

    function test_IMP_D_006_statementFormDeleteTransactionDelegatesToState() {
        const form = createForm()

        findRequired(form, "statementDraftDeleteTransactionButton").clicked()

        compare(statementViewModel.deleteTransactionCalls, 1)
    }

    function test_IMP_D_009_statementFormNameAndAddTransactionDelegateToState() {
        const form = createForm()
        const nameField = findRequired(form, "statementDraftNameField")

        nameField.text = "New Statement"
        nameField.textEdited()
        findRequired(form, "statementDraftAddTransactionButton").clicked()

        compare(statementViewModel.statementName, "New Statement")
        compare(statementViewModel.addTransactionCalls, 1)
    }
}
