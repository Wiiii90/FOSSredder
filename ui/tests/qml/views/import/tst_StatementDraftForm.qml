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

    property var importViewModel: QtObject {
        property bool hasDraft: true
        property string statementName: "Draft Name"
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
            importViewModel: testCase.importViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createForm() {
        return createTemporaryObject(formComponent, testCase)
    }

    function init() {
        importViewModel.hasDraft = true
        importViewModel.statementName = "Draft Name"
        importViewModel.canDeleteTransaction = true
        importViewModel.addTransactionCalls = 0
        importViewModel.deleteTransactionCalls = 0
    }

    function test_IMP_D_006_statementFormDeleteTransactionDelegatesToState() {
        const form = createForm()

        findRequired(form, "statementDraftDeleteTransactionButton").clicked()

        compare(importViewModel.deleteTransactionCalls, 1)
    }

    function test_IMP_D_009_statementFormNameAndAddTransactionDelegateToState() {
        const form = createForm()
        const nameField = findRequired(form, "statementDraftNameField")

        nameField.text = "New Statement"
        nameField.textEdited()
        findRequired(form, "statementDraftAddTransactionButton").clicked()

        compare(importViewModel.statementName, "New Statement")
        compare(importViewModel.addTransactionCalls, 1)
    }
}
