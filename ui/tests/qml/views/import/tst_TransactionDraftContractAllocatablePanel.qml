/**
 * @file ui/tests/qml/views/import/tst_TransactionDraftContractAllocatablePanel.qml
 * @brief Provides QML tests for TransactionDraftContractAllocatablePanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "TransactionDraftContractAllocatablePanelTests"
    when: windowShown
    width: 420
    height: 220

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var importViewModel: QtObject {
        property bool effectiveAllocatable: false
        property real allocatableSuggestionConfidence: 0.8
        property string allocatableSuggestionText: "Allocatable suggestion"
        property int toggleCalls: 0
        function suggestionTone(confidence) { return confidence >= 0.75 ? 2 : 0 }
        function toggleAllocatable() { toggleCalls += 1; effectiveAllocatable = !effectiveAllocatable }
    }

    Component {
        id: panelComponent
        Import.TransactionDraftContractAllocatablePanel {
            width: testCase.width
            theme: testCase.theme
            importViewModel: testCase.importViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_IMP_D_017_allocatableToggleDelegatesToTransactionState() {
        const panel = createTemporaryObject(panelComponent, testCase)

        findRequired(panel, "transactionDraftAllocatableToggle").clicked()

        compare(importViewModel.toggleCalls, 1)
        compare(importViewModel.effectiveAllocatable, true)
    }
}
