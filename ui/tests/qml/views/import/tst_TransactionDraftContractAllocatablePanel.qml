/**
 * @file ui/tests/qml/views/import/tst_TransactionDraftContractAllocatablePanel.qml
 * @brief Provides QML tests for TransactionDraftContractAllocatablePanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "TransactionDraftContractAllocatablePanelTests"
    when: windowShown
    width: 420
    height: 220

    property var theme: QtObject {
        property int spacingSmall: 6
        property int radius: 3
        property int viewCompactActionButtonSize: 28
        property string fontFamily: "Arial"
        property int fontSize: 10
        property color border: "#cccccc"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color textPrimary: "#000000"
        property color danger: "#aa0000"
        property color successStrong: "#008800"
        property color warning: "#aa8800"
    }

    property var transactionState: QtObject {
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
            transactionState: testCase.transactionState
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_IMP_D_017_allocatableToggleDelegatesToTransactionState() {
        const panel = createTemporaryObject(panelComponent, testCase)

        findRequired(panel, "transactionDraftAllocatableToggle").clicked(null)

        compare(transactionState.toggleCalls, 1)
        compare(transactionState.effectiveAllocatable, true)
    }
}
