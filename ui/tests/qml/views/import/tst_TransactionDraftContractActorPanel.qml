/**
 * @file ui/tests/qml/views/import/tst_TransactionDraftContractActorPanel.qml
 * @brief Provides QML tests for TransactionDraftContractActorPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "TransactionDraftContractActorPanelTests"
    when: windowShown
    width: 900
    height: 260

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacingMedium: 8
        property int radius: 3
        property int controlHeight: 32
        property color border: "#cccccc"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color textPrimary: "#000000"
        property color danger: "#aa0000"
        property color successStrong: "#008800"
        property color warning: "#aa8800"
    }

    property var transactionState: QtObject {
        property var actorChoiceModel: [{ id: "", display: "" }, { id: "actor-1", display: "Alice" }]
        property int selectedActorIndex: 0
        property string actorText: ""
        property bool canAddActor: true
        property real actorSuggestionConfidence: 0
        property string actorSuggestionSummary: "0% Confidence - No suggestion"
        property int addActorCalls: 0
        function suggestionTone(confidence) { return confidence >= 0.75 ? 2 : 0 }
        function selectActorIndex(index) { selectedActorIndex = index }
        function addActorFromText() { addActorCalls += 1 }
    }

    Component {
        id: panelComponent
        Import.TransactionDraftContractActorPanel {
            width: testCase.width
            theme: testCase.theme
            transactionState: testCase.transactionState
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function init() {
        transactionState.actorSuggestionConfidence = 0
        transactionState.actorSuggestionSummary = "0% Confidence - No suggestion"
        transactionState.actorText = ""
        transactionState.addActorCalls = 0
    }

    function test_IMP_D_014_actorPanelDelegatesQuickCreate() {
        const panel = createTemporaryObject(panelComponent, testCase)
        const input = findRequired(panel, "transactionDraftActorTextField")

        input.text = "Alice Example"
        input.textEdited()
        findRequired(panel, "transactionDraftActorAddFromTextButton").clicked()

        compare(transactionState.actorText, "Alice Example")
        compare(transactionState.addActorCalls, 1)
    }

    function test_IMP_D_020_actorSuggestionLabelRebindsWhenMatcherStateChanges() {
        const panel = createTemporaryObject(panelComponent, testCase)
        const label = findRequired(panel, "transactionDraftActorSuggestionLabel")

        compare(label.text, "0% Confidence - No suggestion")
        compare(label.color, theme.danger)

        transactionState.actorSuggestionConfidence = 0.8
        transactionState.actorSuggestionSummary = "80% Confidence - Alice"
        wait(0)

        compare(label.text, "80% Confidence - Alice")
        compare(label.color, theme.successStrong)
    }
}
