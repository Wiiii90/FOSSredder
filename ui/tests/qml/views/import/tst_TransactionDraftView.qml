/**
 * @file ui/tests/qml/views/import/tst_TransactionDraftView.qml
 * @brief Provides QML composition tests for TransactionDraftView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "TransactionDraftViewTests"
    when: windowShown
    width: 900
    height: 640

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacingMedium: 8
        property int panelPadding: 12
        property int borderWidthThin: 1
        property int radius: 3
        property int controlHeight: 32
        property int viewCompactActionButtonSize: 28
        property string fontFamily: "Arial"
        property int fontSize: 10
        property color border: "#cccccc"
        property color borderSoft: "#dddddd"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color danger: "#aa0000"
        property color successStrong: "#008800"
        property color warning: "#aa8800"
    }

    property var transactionState: QtObject {
        property string nameText: "Tx 1"
        property string bookingDateText: "2026-05-16"
        property string valutaText: "2026-05-17"
        property string amountText: "12.50"
        property var statusOptions: [{ label: "Open", value: 0 }]
        property int statusIndex: 0
        property string metadataText: "Metadata"
        property string proofSource: ""
        property var actorChoiceModel: [{ id: "", display: "" }]
        property int selectedActorIndex: 0
        property string actorText: ""
        property bool canAddActor: false
        property var contractChoiceModel: [{ id: "", display: "" }]
        property int selectedContractIndex: 0
        property string contractNameText: ""
        property string contractTypeText: ""
        property string contractNamePlaceholder: ""
        property var contractAllocatableModes: [{ label: "Mixed", value: "mixed" }]
        property int contractAllocatableModeIndex: 0
        property string selectedContractType: ""
        property bool canAddContract: false
        property var propertyRows: []
        property string newPropertyName: ""
        property bool canAddProperty: false
        property bool effectiveAllocatable: false
        property real actorSuggestionConfidence: 0
        property string actorSuggestionSummary: "No actor suggestion"
        property real propertySuggestionConfidence: 0
        property string propertySuggestionSummary: "No property suggestion"
        property real contractSuggestionConfidence: 0
        property string contractSuggestionSummary: "No contract suggestion"
        property real allocatableSuggestionConfidence: 0
        property string allocatableSuggestionText: "No allocatable suggestion"
        function commitNameText() {}
        function commitBookingDateText() {}
        function commitValutaText() {}
        function commitAmountText() {}
        function setStatusByIndex(index) {}
        function suggestionTone(confidence) { return 0 }
        function selectActorIndex(index) {}
        function addActorFromText() {}
        function selectContractIndex(index) {}
        function addContractFromFields() {}
        function isPropertySelected(propertyId) { return false }
        function setPropertySelected(propertyId, selected) {}
        function addPropertyFromInput() {}
        function toggleAllocatable() {}
    }

    Component {
        id: viewComponent
        Import.TransactionDraftView {
            width: testCase.width
            theme: testCase.theme
            transactionState: testCase.transactionState
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_IMP_D_019_transactionDraftViewComposesPanels() {
        const view = createTemporaryObject(viewComponent, testCase)

        verify(findRequired(view, "transactionDraftNameField") !== null)
        verify(findRequired(view, "transactionDraftMetadataTextArea") !== null)
        verify(findRequired(view, "transactionDraftContractNameField") !== null)
    }
}
