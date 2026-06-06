/**
 * @file ui/tests/qml/views/contract/tst_ContractSidebar.qml
 * @brief Provides QML tests for ContractSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Contract 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ContractSidebarTests"
    when: windowShown
    width: 320
    height: 240

    property var workspaceRoles: QtObject {
        property string selectedContractId: ""
        property var contractRows: []
        property var contractViewModel: QtObject {
            readonly property string currentId: testCase.workspaceRoles.selectedContractId
            property var contractRows: testCase.workspaceRoles.contractRows
            property string lastSelectedId: ""
            function selectContract(id) {
                lastSelectedId = String(id || "")
                testCase.workspaceRoles.selectedContractId = lastSelectedId
            }
        }
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: sidebarComponent
        ContractSidebar {
            width: 320
            height: 240
            contractViewModel: testCase.workspaceRoles.contractViewModel
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
        workspaceRoles.selectedContractId = ""
        workspaceRoles.contractRows = []
        workspaceRoles.contractViewModel.lastSelectedId = ""
    }

    function test_CON_S_001_rowsRenderFromWorkspaceRoles() {
        workspaceRoles.contractRows = [
            { id: "contract-1", name: "Lease 1" },
            { id: "contract-2", name: "Lease 2" }
        ]

        const sidebar = createSidebar()
        compare(findRequired(sidebar, "contractSidebarRow_contract-1").height, theme.viewSidebarRowHeight)
        compare(findRequired(sidebar, "contractSidebarName_contract-2").text, "Lease 2")
    }

    function test_CON_S_002_rowClickWritesSelectedContractId() {
        workspaceRoles.contractRows = [
            { id: "contract-1", name: "Lease 1" },
            { id: "contract-2", name: "Lease 2" }
        ]

        const sidebar = createSidebar()
        const mouseArea = findRequired(sidebar, "contractSidebarMouse_contract-2")

        mouseArea.clicked(null)

        compare(workspaceRoles.selectedContractId, "contract-2")
        compare(workspaceRoles.contractViewModel.lastSelectedId, "contract-2")
    }

    function test_CON_S_003_selectedRowUsesThemeHighlight() {
        workspaceRoles.selectedContractId = "contract-2"
        workspaceRoles.contractRows = [
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
        workspaceRoles.contractRows = rows

        const sidebar = createSidebar()
        const flick = findRequired(sidebar, "contractSidebarFlick")
        wait(50)

        compare(flick.contentHeight > flick.height, true)
    }
}
