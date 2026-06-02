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

    property var transactionViewModel: QtObject {
        property string nameText: "Tx 1"
        property string bookingDateText: "2026-05-16"
        property string valutaText: "2026-05-17"
        property string amountText: "12.50"
        property var statusOptions: [{ label: "Open", value: 0 }]
        property int statusIndex: 0
        property string metadataText: "Metadata"
        property string proofSource: ""
        property var actorOptions: [{ id: "", display: "" }]
        property int selectedActorOptionIndex: 0
        property string actorName: ""
        property bool canAddActor: false
        property var contractOptions: [{ id: "", display: "" }]
        property int selectedContractOptionIndex: 0
        property string contractName: ""
        property string contractType: ""
        property string contractNamePlaceholder: ""
        property var contractAllocatableModes: [{ label: "Mixed", value: "mixed" }]
        property int contractAllocatableModeIndex: 0
        property string selectedContractType: ""
        property bool canAddContract: false
        property var propertyOptions: []
        property string propertyName: ""
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
        function selectStatusAtIndex(index) {}
        function suggestionTone(confidence) { return 0 }
        function selectActorAtIndex(index) {}
        function addActor() {}
        function selectContractAtIndex(index) {}
        function addContract() {}
        function isPropertySelected(propertyId) { return false }
        function setPropertySelected(propertyId, selected) {}
        function addProperty() {}
        function toggleAllocatable() {}
    }

    Component {
        id: viewComponent
        Import.TransactionDraftView {
            width: testCase.width
            theme: testCase.theme
            transactionViewModel: testCase.transactionViewModel
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
