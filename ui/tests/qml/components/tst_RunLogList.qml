/**
 * @file ui/tests/qml/components/tst_RunLogList.qml
 * @brief Provides QML tests for RunLogList behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../common" as Common
import FossRedder.Components 1.0

import "../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "RunLogListTests"
    when: windowShown
    width: 720
    height: 480

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

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
        control.deleteClicked.connect(function(index, logId, draftAttached, draftId) {
            received = { index: index, logId: logId, draftAttached: draftAttached, draftId: draftId }
        })
        wait(0)

        const deleteButton = findRequired(control, "runLogDelete_log-1")
        deleteButton.clicked()

        compare(received.index, 0)
        compare(received.logId, "log-1")
        compare(received.draftAttached, false)
    }

}
