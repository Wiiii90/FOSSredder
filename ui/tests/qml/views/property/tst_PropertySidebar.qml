/**
 * @file ui/tests/qml/views/property/tst_PropertySidebar.qml
 * @brief Provides QML tests for PropertySidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "PropertySidebarTests"
    when: windowShown
    width: 320
    height: 240

    property var session: QtObject {
        property string selectedPropertyId: ""
        property var propertyRows: []
        property var propertyState: QtObject {
            readonly property string currentId: testCase.session.selectedPropertyId
            property var propertyRows: testCase.session.propertyRows
            property string lastSelectedId: ""
            function selectProperty(id) {
                lastSelectedId = String(id || "")
                testCase.session.selectedPropertyId = lastSelectedId
            }
        }
    }

    property var theme: QtObject {
        property int spacingMedium: 8
        property int spacingSmall: 6
        property int viewSidebarRowHeight: 40
        property int viewSidebarRowRadius: 4
        property int viewSidebarRowSpacing: 4
        property int borderWidthThin: 1
        property color selectionHighlight: "#aaccee"
        property color borderSoft: "#cccccc"
        property color textPrimary: "#000000"
    }

    Component {
        id: sidebarComponent
        PropertySidebar {
            width: 320
            height: 240
            propertyState: testCase.session.propertyState
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
        session.selectedPropertyId = ""
        session.propertyRows = []
    }

    function test_PROP_S_001_rowsRenderFromWorkspaceFacade() {
        session.propertyRows = [
            { id: "property-1", name: "Flat 1" },
            { id: "property-2", name: "Flat 2" }
        ]

        const sidebar = createSidebar()
        compare(findRequired(sidebar, "propertySidebarRow_property-1").height, theme.viewSidebarRowHeight)
        compare(findRequired(sidebar, "propertySidebarName_property-2").text, "Flat 2")
    }

    function test_PROP_S_002_rowClickWritesSelectedPropertyId() {
        session.propertyRows = [
            { id: "property-1", name: "Flat 1" },
            { id: "property-2", name: "Flat 2" }
        ]

        const sidebar = createSidebar()
        const mouseArea = findRequired(sidebar, "propertySidebarMouse_property-2")

        mouseArea.clicked(null)

        compare(session.selectedPropertyId, "property-2")
        compare(session.propertyState.lastSelectedId, "property-2")
    }

    function test_PROP_S_003_selectedRowUsesThemeHighlight() {
        session.selectedPropertyId = "property-2"
        session.propertyRows = [
            { id: "property-1", name: "Flat 1" },
            { id: "property-2", name: "Flat 2" }
        ]

        const sidebar = createSidebar()
        compare(findRequired(sidebar, "propertySidebarRow_property-2").color, theme.selectionHighlight)
    }

    function test_PROP_S_004_sidebarScrollsWhenRowsExceedViewport() {
        const rows = []
        for (let i = 0; i < 12; ++i)
            rows.push({ id: "property-" + i, name: "Property " + i })
        session.propertyRows = rows

        const sidebar = createSidebar()
        const flick = findRequired(sidebar, "propertySidebarFlick")
        wait(50)

        compare(flick.contentHeight > flick.height, true)
    }

    function test_PROP_S_005_rowsRefreshWhenWorkspaceRowsChange() {
        session.propertyRows = [
            { id: "property-1", name: "Flat 1" }
        ]

        const sidebar = createSidebar()
        compare(findRequired(sidebar, "propertySidebarName_property-1").text, "Flat 1")

        session.propertyRows = [
            { id: "property-1", name: "Flat 1" },
            { id: "property-2", name: "Flat 2" }
        ]
        wait(50)

        compare(findRequired(sidebar, "propertySidebarName_property-2").text, "Flat 2")
    }
}
