/**
 * @file ui/tests/qml/views/contract/tst_ContractSidebar.qml
 * @brief Provides QML tests for ContractSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ContractSidebarTests"
    when: windowShown
    width: 320
    height: 240

    property var workspaceFacade: QtObject {
        property string selectedContractId: ""
        property var contractRows: []
        property var contractViewModel: QtObject {
            readonly property string currentId: testCase.workspaceFacade.selectedContractId
            property var contractRows: testCase.workspaceFacade.contractRows
            property string lastSelectedId: ""
            function selectContract(id) {
                lastSelectedId = String(id || "")
                testCase.workspaceFacade.selectedContractId = lastSelectedId
            }
        }
    }

    property var theme: QtObject {
        property int spacingMedium: 8
        property int spacingSmall: 6
        property int viewSidebarRowHeight: 40
        property int viewSidebarRowRadius: 4
        property int viewSidebarRowSpacing: 4
        property int viewSidebarEntryInset: 6
        property int viewSidebarScrollBarOuterInset: 6
        property int viewSidebarEntryInsetTotal: 12
        property int borderWidthThin: 1
        property color selectionHighlight: "#aaccee"
        property color selectionBorder: "#aaccee"
        property color borderSoft: "#cccccc"
        property color sidebarHoverFill: "#eeeeee"
        property color sidebarHoverBorder: "#bbbbbb"
        property color textPrimary: "#000000"
    }

    Component {
        id: sidebarComponent
        ContractSidebar {
            width: 320
            height: 240
            contractViewModel: testCase.workspaceFacade.contractViewModel
            theme: testCase.theme
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createSidebar() {
        return createTemporaryObject(sidebarComponent, testCase)
    }

    function init() {
        workspaceFacade.selectedContractId = ""
        workspaceFacade.contractRows = []
        workspaceFacade.contractViewModel.lastSelectedId = ""
    }

    function test_CON_S_001_rowsRenderFromWorkspaceFacade() {
        workspaceFacade.contractRows = [
            { id: "contract-1", name: "Lease 1" },
            { id: "contract-2", name: "Lease 2" }
        ]

        const sidebar = createSidebar()
        compare(findRequired(sidebar, "contractSidebarRow_contract-1").height, theme.viewSidebarRowHeight)
        compare(findRequired(sidebar, "contractSidebarName_contract-2").text, "Lease 2")
    }

    function test_CON_S_002_rowClickWritesSelectedContractId() {
        workspaceFacade.contractRows = [
            { id: "contract-1", name: "Lease 1" },
            { id: "contract-2", name: "Lease 2" }
        ]

        const sidebar = createSidebar()
        const mouseArea = findRequired(sidebar, "contractSidebarMouse_contract-2")

        mouseArea.clicked(null)

        compare(workspaceFacade.selectedContractId, "contract-2")
        compare(workspaceFacade.contractViewModel.lastSelectedId, "contract-2")
    }

    function test_CON_S_003_selectedRowUsesThemeHighlight() {
        workspaceFacade.selectedContractId = "contract-2"
        workspaceFacade.contractRows = [
            { id: "contract-1", name: "Lease 1" },
            { id: "contract-2", name: "Lease 2" }
        ]

        const sidebar = createSidebar()
        compare(findRequired(sidebar, "contractSidebarRow_contract-2").color, theme.selectionHighlight)
    }

    function test_CON_S_004_sidebarScrollsWhenRowsExceedViewport() {
        const rows = []
        for (let i = 0; i < 12; ++i)
            rows.push({ id: "contract-" + i, name: "Contract " + i })
        workspaceFacade.contractRows = rows

        const sidebar = createSidebar()
        const flick = findRequired(sidebar, "contractSidebarFlick")
        wait(50)

        compare(flick.contentHeight > flick.height, true)
    }
}
