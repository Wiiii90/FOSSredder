/**
 * @file ui/tests/qml/views/import/tst_TransactionDraftContractPanel.qml
 * @brief Provides QML tests for TransactionDraftContractPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Import 1.0 as Import

import "../../common"

TestCase {
    id: testCase
    name: "TransactionDraftContractPanelTests"
    when: windowShown
    width: 900
    height: 520

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var importViewModel: QtObject {
        property string actorName: ""
        property var actorOptions: [{ id: "", display: "" }]
        property int selectedActorOptionIndex: 0
        property bool canAddActor: true
        property var contractOptions: [{ id: "", display: "" }, { id: "contract-1", display: "Lease", type: "Rent" }]
        property int selectedContractOptionIndex: 0
        property string contractName: ""
        property string contractType: "Lease"
        property string contractNamePlaceholder: "Contract 1"
        property var contractAllocatableModes: [{ label: "Mixed", value: "mixed" }, { label: "Yes", value: "true" }]
        property int contractAllocatableModeIndex: 0
        property string selectedContractType: ""
        property bool canAddContract: true
        property var propertyOptions: []
        property string propertyName: ""
        property bool canAddProperty: false
        property bool effectiveAllocatable: false
        property real actorSuggestionConfidence: 0.8
        property string actorSuggestionSummary: "Actor suggestion"
        property real propertySuggestionConfidence: 0.8
        property string propertySuggestionSummary: "Property suggestion"
        property real contractSuggestionConfidence: 0.8
        property string contractSuggestionSummary: "Contract suggestion"
        property real allocatableSuggestionConfidence: 0.8
        property string allocatableSuggestionText: "Allocatable suggestion"
        property int addContractCalls: 0
        function suggestionTone(confidence) { return confidence >= 0.75 ? 2 : 0 }
        function selectActorAtIndex(index) { selectedActorOptionIndex = index }
        function addActor() {}
        function selectContractAtIndex(index) { selectedContractOptionIndex = index }
        function addContract() { addContractCalls += 1 }
        function isPropertySelected(propertyId) { return false }
        function setPropertySelected(propertyId, selected) {}
        function addProperty() {}
        function toggleAllocatable() { effectiveAllocatable = !effectiveAllocatable }
    }

    Component {
        id: panelComponent
        Import.TransactionDraftContractPanel {
            width: testCase.width
            theme: testCase.theme
            importViewModel: testCase.importViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_IMP_D_013_contractPanelDelegatesQuickCreate() {
        const panel = createTemporaryObject(panelComponent, testCase)
        const nameField = findRequired(panel, "transactionDraftContractNameField")
        const typeField = findRequired(panel, "transactionDraftContractTypeField")

        nameField.text = "Office Rent"
        nameField.textEdited()
        typeField.text = "Rent"
        typeField.textEdited()
        findRequired(panel, "transactionDraftContractAddButton").clicked()

        compare(importViewModel.contractName, "Office Rent")
        compare(importViewModel.contractType, "Rent")
        compare(importViewModel.addContractCalls, 1)
    }
}
