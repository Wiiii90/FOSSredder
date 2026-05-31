/**
 * @file ui/tests/qml/components/tst_RunLogList.qml
 * @brief Provides QML tests for RunLogList behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Components 1.0

import "../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "RunLogListTests"
    when: windowShown
    width: 720
    height: 480

    property var theme: QtObject {
        property int spacingSmall: 6
        property int borderWidthThin: 1
        property int radius: 3
        property int viewSidebarEntryInset: 6
        property int viewSidebarScrollBarOuterInset: 6
        property int viewSidebarEntryInsetTotal: 12
        property int viewCompactActionButtonSize: 28
        property int viewAliasChipHeight: 30
        property int viewAliasChipRadius: 4
        property int margins: 2
        property int fontSizeSmall: 10
        property string fontFamily: "Arial"
        property color accent: "#3366ff"
        property color border: "#cccccc"
        property color borderSoft: "#cccccc"
        property color selectionHighlight: "#ffd39c"
        property color selectionBorder: "#cc9933"
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
        id: runLogListComponent
        RunLogList {
            width: 700
            height: 420
            theme: testCase.theme
            model: [
                {
                    logId: "log-1",
                    time: "2026-01-01T10:00:00Z",
                    status: "Success",
                    file: "test:///exports/a.xlsx",
                    message: "done",
                    payload: '{"items":[{"objectType":"Annual"},{"objectType":"Analysis","exportType":"CSV"}]}',
                    displayTime: "2026-01-01 10:00",
                    displayTitle: "Export 'Rent Overview'",
                    displayStatusDetail: "done",
                    draftAttached: false,
                    draftId: "",
                    statementId: "statement-1"
                }
            ]
        }
    }

    function createControl() {
        return createTemporaryObject(runLogListComponent, testCase)
    }

    function findRequired(root, objectName) {
        const found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function test_CTRL_RL_003_displayRolesAreRendered() {
        const control = createControl()
        wait(0)

        compare(findRequired(control, "runLogTitle_log-1").text, "Export 'Rent Overview'")
        compare(findRequired(control, "runLogDetail_log-1").text, "done")
        compare(findRequired(control, "runLogTime_log-1").text, "2026-01-01 10:00")
    }

    function test_CTRL_RL_007_detailLineKeepsReservedHeightWhenMessageMissing() {
        const control = createTemporaryObject(runLogListComponent, testCase, {
            model: [
                {
                    logId: "log-empty-detail",
                    time: "2026-01-01T10:00:00Z",
                    status: "Success",
                    file: "test:///exports/a.xlsx",
                    message: "",
                    payload: "",
                    displayTime: "2026-01-01 10:00",
                    displayTitle: "a.xlsx",
                    displayStatusDetail: "",
                    draftAttached: false,
                    draftId: "",
                    statementId: ""
                },
                {
                    logId: "log-with-detail",
                    time: "2026-01-01T10:00:00Z",
                    status: "Success",
                    file: "test:///exports/b.xlsx",
                    message: "Draft was finalized into a statement.",
                    payload: "",
                    displayTime: "2026-01-01 10:00",
                    displayTitle: "b.xlsx",
                    displayStatusDetail: "Draft was finalized into a statement.",
                    draftAttached: false,
                    draftId: "",
                    statementId: ""
                }
            ]
        })
        wait(0)

        const emptyCard = findRequired(control, "runLogCard_log-empty-detail")
        const filledCard = findRequired(control, "runLogCard_log-with-detail")
        compare(emptyCard.height, filledCard.height)
    }

    function test_CTRL_RL_001_logClickEmitsRunSignal() {
        const control = createControl()
        let received = ({})
        control.runClicked.connect(function(index, logId, draftAttached, statementId, draftId) {
            received = { index: index, logId: logId, draftAttached: draftAttached, statementId: statementId, draftId: draftId }
        })
        wait(0)

        const mouseArea = findRequired(control, "runLogRow_log-1")
        mouseArea.clicked(null)

        compare(received.index, 0)
        compare(received.logId, "log-1")
        compare(received.statementId, "statement-1")
    }

    function test_CTRL_RL_005_delegateHasClickableHeight() {
        const control = createControl()
        wait(0)

        const mouseArea = findRequired(control, "runLogRow_log-1")
        verify(mouseArea.height > 0)
    }

    function test_CTRL_RL_006_deletedRunWithoutStatementIsNotClickable() {
        const control = createTemporaryObject(runLogListComponent, testCase, {
            model: [
                {
                    logId: "deleted-log",
                    time: "2026-01-01T10:00:00Z",
                    status: "Deleted",
                    file: "test:///imports/a.pdf",
                    message: "Imported statement was deleted.",
                    payload: "",
                    displayTime: "2026-01-01 10:00",
                    displayTitle: "a.pdf",
                    displayStatusDetail: "Imported statement was deleted.",
                    draftAttached: false,
                    draftId: "",
                    statementId: ""
                }
            ]
        })
        wait(0)

        const mouseArea = findRequired(control, "runLogRow_deleted-log")
        verify(!mouseArea.enabled)
    }

    function test_CTRL_RL_004_clickEmitsDraftId() {
        const control = createTemporaryObject(runLogListComponent, testCase, {
            model: [
                {
                    logId: "draft-log",
                    time: "2026-01-01T10:00:00Z",
                    status: "Draft",
                    file: "test:///imports/a.pdf",
                    message: "draft",
                    payload: "",
                    displayTime: "2026-01-01 10:00",
                    displayTitle: "a.pdf",
                    displayStatusDetail: "draft",
                    draftAttached: true,
                    draftId: "draft-1",
                    statementId: ""
                }
            ]
        })
        let received = ({})
        control.runClicked.connect(function(index, logId, draftAttached, statementId, draftId) {
            received = { index: index, logId: logId, draftAttached: draftAttached, statementId: statementId, draftId: draftId }
        })
        wait(0)

        const mouseArea = findRequired(control, "runLogRow_draft-log")
        verify(mouseArea.enabled)
        mouseArea.clicked(null)

        compare(received.logId, "draft-log")
        compare(received.draftAttached, true)
        compare(received.draftId, "draft-1")
    }

    function test_CTRL_RL_002_deleteClickEmitsDeleteSignal() {
        const control = createControl()
        let received = ({})
        control.deleteClicked.connect(function(index, draftAttached, draftId) {
            received = { index: index, draftAttached: draftAttached, draftId: draftId }
        })
        wait(0)

        const deleteButton = findRequired(control, "runLogDelete_log-1")
        deleteButton.clicked()

        compare(received.index, 0)
        compare(received.draftAttached, false)
    }

}
