/**
 * @file ui/tests/qml/views/actor/tst_ActorSidebar.qml
 * @brief Provides QML tests for ActorSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ActorSidebarTests"
    when: windowShown
    width: 320
    height: 240

    property var session: QtObject {
        property string selectedActorId: ""
        property var actorRows: []
        property var actorState: QtObject {
            readonly property string currentId: testCase.session.selectedActorId
            property var actorRows: testCase.session.actorRows
            property string lastSelectedId: ""
            function selectActor(id) {
                lastSelectedId = String(id || "")
                testCase.session.selectedActorId = lastSelectedId
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
        ActorSidebar {
            width: 320
            height: 240
            actorState: testCase.session.actorState
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
        session.selectedActorId = ""
        session.actorRows = []
    }

    function test_ACT_S_001_rowsRenderFromWorkspaceFacade() {
        session.actorRows = [
            { id: "actor-1", name: "Alice" },
            { id: "actor-2", name: "Bob" }
        ]

        const sidebar = createSidebar()
        compare(findRequired(sidebar, "actorSidebarRow_actor-1").height, theme.viewSidebarRowHeight)
        compare(findRequired(sidebar, "actorSidebarName_actor-2").text, "Bob")
    }

    function test_ACT_S_002_rowClickWritesSelectedActorId() {
        session.actorRows = [
            { id: "actor-1", name: "Alice" },
            { id: "actor-2", name: "Bob" }
        ]

        const sidebar = createSidebar()
        const mouseArea = findRequired(sidebar, "actorSidebarMouse_actor-2")

        mouseArea.clicked(null)

        compare(session.selectedActorId, "actor-2")
        compare(session.actorState.lastSelectedId, "actor-2")
    }

    function test_ACT_S_003_selectedRowUsesThemeHighlight() {
        session.selectedActorId = "actor-2"
        session.actorRows = [
            { id: "actor-1", name: "Alice" },
            { id: "actor-2", name: "Bob" }
        ]

        const sidebar = createSidebar()
        compare(findRequired(sidebar, "actorSidebarRow_actor-2").color, theme.selectionHighlight)
    }

    function test_ACT_S_004_sidebarScrollsWhenRowsExceedViewport() {
        const rows = []
        for (let i = 0; i < 12; ++i)
            rows.push({ id: "actor-" + i, name: "Actor " + i })
        session.actorRows = rows

        const sidebar = createSidebar()
        const flick = findRequired(sidebar, "actorSidebarFlick")
        wait(50)

        compare(flick.contentHeight > flick.height, true)
    }
}
