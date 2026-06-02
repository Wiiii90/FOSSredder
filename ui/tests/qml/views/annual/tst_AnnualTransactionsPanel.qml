/**
 * @file ui/tests/qml/views/annual/tst_AnnualTransactionsPanel.qml
 * @brief Provides QML tests for AnnualTransactionsPanel category rendering.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Annual 1.0 as Annual

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnnualTransactionsPanelTests"
    when: windowShown
    width: 960
    height: 640

    property var annualViewModel: QtObject {
        property var annualTransactions: [{ id: "d-1" }]
        property var transactionSections: [
            { key: "deduplicated", title: "Included entries (exact matches)", expanded: true, visible: true, rows: [row("d-1", "Dedup", 0)] },
            { key: "similar", title: "Included entries (possible variants)", expanded: true, visible: true, rows: [row("s-1", "Similar", 1)] },
            { key: "divergent", title: "Included entries (unique)", expanded: true, visible: true, rows: [row("v-1", "Divergent", 2)] },
            { key: "workspaceOnly", title: "Missing live transactions from selected year", expanded: true, visible: true, rows: [row("w-1", "WorkspaceOnly", 3)] },
            { key: "missingLive", title: "Included deleted transactions", expanded: true, visible: true, rows: [row("m-1", "MissingLive", 0)] }
        ]
        property string toggledKey: ""
        function row(id, name, status) {
            return {
                id: id,
                name: name,
                sourceNamesText: "",
                bookingDate: "2026-01-01",
                amountText: "10.00",
                allocatableText: "Allocatable",
                allocatable: true,
                contractType: "rent",
                contractTypeLabel: "rent",
                status: status,
                statusText: "Neutral",
                statusTone: "primary",
                isMixedYear: false
            }
        }
        function toggleTransactionSection(key) { toggledKey = key }
    }

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacing: 8
        property int controlHeight: 32
        property int viewNavigationButtonWidth: 42
        property int viewCompactActionButtonSizeSmall: 32
        property int radius: 3
        property int borderWidthThin: 1
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color success: "#0a7f2e"
        property color warning: "#a86d00"
        property color danger: "#b0302f"
        property color info: "#1a73b8"
        property var annual: ({
            transactions: {
                tableMinWidth: 720,
                dateColumnWidth: 110,
                amountColumnWidth: 90,
                allocatableColumnWidth: 130,
                typeColumnWidth: 120,
                statusColumnWidth: 100
            }
        })
    }

    Component {
        id: panelComponent
        Annual.AnnualTransactionsPanel {
            width: 960
            height: 640
            theme: testCase.theme
            annualViewModel: testCase.annualViewModel
        }
    }

    function createPanel() {
        return createTemporaryObject(panelComponent, testCase)
    }

    function test_ANN_T_001_rendersAllCategoryTogglesWhenRowsExist() {
        const panel = createPanel()
        wait(0)

        verify(TestSupport.findRequired(Lookup, panel, "annualTransactionsSectionToggle_deduplicated") !== null)
        verify(TestSupport.findRequired(Lookup, panel, "annualTransactionsSectionToggle_similar") !== null)
        verify(TestSupport.findRequired(Lookup, panel, "annualTransactionsSectionToggle_divergent") !== null)
        verify(TestSupport.findRequired(Lookup, panel, "annualTransactionsSectionToggle_workspaceOnly") !== null)
        verify(TestSupport.findRequired(Lookup, panel, "annualTransactionsSectionToggle_missingLive") !== null)
    }

    function test_ANN_T_002_sectionToggleDelegatesToAnnualState() {
        const panel = createPanel()
        TestSupport.findRequired(Lookup, panel, "annualTransactionsSectionMouseArea_similar").clicked(null)
        compare(annualViewModel.toggledKey, "similar")
    }
}
