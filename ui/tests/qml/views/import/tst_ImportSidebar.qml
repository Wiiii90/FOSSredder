/**
 * @file ui/tests/qml/views/import/tst_ImportSidebar.qml
 * @brief Provides QML tests for ImportSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ImportSidebarTests"
    when: windowShown
    width: 960
    height: 640

    property var importWorkflow: QtObject {
        property var logs: []
    }

    property var importViewModel: QtObject {
        property var importWorkflow: testCase.importWorkflow
        property var importLogs: testCase.importWorkflow.logs
        property string selectedDraftId: ""
        property int openCalls: 0
        property int deleteCalls: 0
        property var lastOpen: ({})
        property var lastDelete: ({})
        function openImportLog(logId, draftAttached, statementId, draftId) {
            openCalls += 1
            lastOpen = { logId: logId, draftAttached: draftAttached, statementId: statementId, draftId: draftId }
        }
        function deleteImportLog(logId, draftAttached, draftId) {
            deleteCalls += 1
            lastDelete = { logId: logId, draftAttached: draftAttached, draftId: draftId }
        }
    }

    property var theme: QtObject {
        property int spacingMedium: 8
        property int spacingSmall: 6
        property int borderWidthThin: 1
        property int radius: 3
        property int viewSidebarRowHeight: 64
        property int viewSidebarRowRadius: 3
        property int viewSidebarEntryInset: 6
        property int viewSidebarScrollBarOuterInset: 6
        property int viewSidebarEntryInsetTotal: 12
        property int viewCompactActionButtonSize: 28
        property int viewCompactActionButtonSizeTiny: 24
        property int viewAliasChipHeight: 30
        property int viewAliasChipRadius: 4
        property int margins: 2
        property int fontSizeSmall: 10
        property int fontSize: 10
        property string fontFamily: "Arial"
        property color accent: "#3366ff"
        property color selectionHighlight: "#aaccee"
        property color selectionBorder: "#aaccee"
        property color border: "#cccccc"
        property color borderSoft: "#cccccc"
        property color sidebarHoverFill: "#eeeeee"
        property color sidebarHoverBorder: "#bbbbbb"
        property color success: "#008800"
        property color warning: "#aa8800"
        property color danger: "#aa0000"
        property color statusSuccessFill: "#e8f5e9"
        property color statusSuccessText: "#008800"
        property color statusSuccessBorder: "#008800"
        property color statusWarningFill: "#fff8e1"
        property color statusWarningText: "#aa8800"
        property color statusWarningBorder: "#aa8800"
        property color statusDangerFill: "#ffebee"
        property color statusDangerText: "#aa0000"
        property color statusDangerBorder: "#aa0000"
        property color statusInfoFill: "#e3f2fd"
        property color statusInfoText: "#3366ff"
        property color statusInfoBorder: "#3366ff"
        property color statusNeutralFill: "#eeeeee"
        property color statusNeutralText: "#666666"
        property color statusNeutralBorder: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: importSidebarComponent
        Import.ImportSidebar {
            width: 960
            height: 640
            importViewModel: testCase.importViewModel
            theme: testCase.theme
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createSidebar() {
        return createTemporaryObject(importSidebarComponent, testCase)
    }

    function init() {
        importWorkflow.logs = [
            { logId: "import-1", time: "2026-05-16 10:00:00", status: "Success", file: "/tmp/import.pdf", message: "done", payload: "", displayTime: "2026-05-16 10:00:00", displayTitle: "import.pdf", displayStatusDetail: "done", draftAttached: false, draftId: "", statementId: "statement-1" }
        ]
        importViewModel.openCalls = 0
        importViewModel.deleteCalls = 0
        importViewModel.lastOpen = ({})
        importViewModel.lastDelete = ({})
    }

    function test_IMP_S_001_importLogBindingShowsImportedLogs() {
        const sidebar = createSidebar()

        compare(findRequired(sidebar, "runLogList").count, 1)
    }

    function test_IMP_S_002_draftImportLogClickDelegatesRowPayload() {
        importWorkflow.logs = [
            { logId: "import-2", time: "2026-05-16 10:00:00", status: "Draft", file: "/tmp/import.pdf", message: "draft", payload: "", displayTime: "2026-05-16 10:00:00", displayTitle: "import.pdf", displayStatusDetail: "draft", draftAttached: true, draftId: "draft-2", statementId: "" }
        ]
        const sidebar = createSidebar()
        const row = findRequired(sidebar, "runLogRow_import-2")

        row.clicked(null)

        compare(importViewModel.openCalls, 1)
        compare(importViewModel.lastOpen.logId, "import-2")
        compare(importViewModel.lastOpen.draftId, "draft-2")
        compare(importViewModel.lastOpen.draftAttached, true)
    }

    function test_IMP_S_003_finalizedImportLogClickDelegatesStatementPayload() {
        importWorkflow.logs = [
            { logId: "import-3", time: "2026-05-16 10:00:00", status: "Finalized", file: "/tmp/import.pdf", message: "done", payload: "", displayTime: "2026-05-16 10:00:00", displayTitle: "import.pdf", displayStatusDetail: "done", draftAttached: false, draftId: "", statementId: "statement-3" }
        ]
        const sidebar = createSidebar()
        const row = findRequired(sidebar, "runLogRow_import-3")

        row.clicked(null)

        compare(importViewModel.openCalls, 1)
        compare(importViewModel.lastOpen.statementId, "statement-3")
    }

    function test_IMP_S_005_deleteDelegatesRowPayload() {
        importWorkflow.logs = [
            { logId: "import-4", time: "2026-05-16 10:00:00", status: "Draft", file: "/tmp/import.pdf", message: "draft", payload: "", displayTime: "2026-05-16 10:00:00", displayTitle: "import.pdf", displayStatusDetail: "draft", draftAttached: true, draftId: "draft-4", statementId: "" }
        ]
        const sidebar = createSidebar()

        findRequired(sidebar, "runLogDelete_import-4").clicked()

        compare(importViewModel.deleteCalls, 1)
        compare(importViewModel.lastDelete.logId, "import-4")
        compare(importViewModel.lastDelete.draftId, "draft-4")
    }

    function test_IMP_S_006_selectedDraftImportLogIsHighlighted() {
        importWorkflow.logs = [
            { logId: "import-5", time: "2026-05-16 10:00:00", status: "Draft", file: "/tmp/import.pdf", message: "draft", payload: "", displayTime: "2026-05-16 10:00:00", displayTitle: "import.pdf", displayStatusDetail: "draft", draftAttached: true, draftId: "draft-5", statementId: "" }
        ]
        importViewModel.selectedDraftId = "draft-5"

        const sidebar = createSidebar()
        const selectedCard = findRequired(sidebar, "runLogCard_import-5")

        compare(selectedCard.border.color, theme.selectionBorder)
    }
}
