/**
 * @file ui/tests/qml/views/import/tst_TransactionDraftContractActorPanel.qml
 * @brief Provides QML tests for TransactionDraftContractActorPanel behavior.
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
    name: "TransactionDraftContractActorPanelTests"
    when: windowShown
    width: 900
    height: 260

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var importViewModel: QtObject {
        property var actorOptions: [{ id: "", display: "" }, { id: "actor-1", display: "Alice" }]
        property int selectedActorOptionIndex: 0
        property string actorName: ""
        property bool canAddActor: true
        property real actorSuggestionConfidence: 0
        property string actorSuggestionSummary: "0% Confidence - No suggestion"
        property int addActorCalls: 0
        function suggestionTone(confidence) { return confidence >= 0.75 ? 2 : 0 }
        function selectActorAtIndex(index) { selectedActorOptionIndex = index }
        function addActor() { addActorCalls += 1 }
    }

    Component {
        id: panelComponent
        Import.TransactionDraftContractActorPanel {
            width: testCase.width
            theme: testCase.theme
            importViewModel: testCase.importViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function init() {
        importViewModel.actorSuggestionConfidence = 0
        importViewModel.actorSuggestionSummary = "0% Confidence - No suggestion"
        importViewModel.actorName = ""
        importViewModel.addActorCalls = 0
    }

    function test_IMP_D_014_actorPanelDelegatesQuickCreate() {
        const panel = createTemporaryObject(panelComponent, testCase)
        const input = findRequired(panel, "transactionDraftActorNameField")

        input.text = "Alice Example"
        input.textEdited()
        findRequired(panel, "transactionDraftActorAddFromTextButton").clicked()

        compare(importViewModel.actorName, "Alice Example")
        compare(importViewModel.addActorCalls, 1)
    }

    function test_IMP_D_020_actorSuggestionLabelRebindsWhenMatcherStateChanges() {
        const panel = createTemporaryObject(panelComponent, testCase)
        const label = findRequired(panel, "transactionDraftActorSuggestionLabel")

        compare(label.text, "0% Confidence - No suggestion")
        compare(label.color, theme.danger)

        importViewModel.actorSuggestionConfidence = 0.8
        importViewModel.actorSuggestionSummary = "80% Confidence - Alice"
        wait(0)

        compare(label.text, "80% Confidence - Alice")
        compare(label.color, theme.successStrong)
    }
}
