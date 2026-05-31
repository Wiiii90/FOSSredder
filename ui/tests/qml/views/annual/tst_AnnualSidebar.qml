/**
 * @file ui/tests/qml/views/annual/tst_AnnualSidebar.qml
 * @brief Provides QML tests for AnnualSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Annual 1.0 as Annual

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnnualSidebarTests"
    when: windowShown
    width: 320
    height: 480

    property var annualState: QtObject {
        property var annualRows: [
            { id: "annual-1", display: "Annual 1", year: 2026 },
            { id: "annual-2", display: "Annual 2", year: 2027 }
        ]
        property string selectedAnnualId: "annual-1"
        property string selectedByClick: ""
        function selectAnnual(id) { selectedByClick = id; selectedAnnualId = id }
    }

    property var theme: QtObject {
        property int shellSidebarPreferredWidth: 320
        property int spacingMedium: 8
        property int spacingSmall: 6
        property int viewSidebarRowHeight: 44
        property int viewSidebarRowRadius: 6
        property int viewSidebarRowSpacing: 2
        property int borderWidthThin: 1
        property color borderSoft: "#cccccc"
        property color selectionHighlight: "#ffd39c"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: sidebarComponent
        Annual.AnnualSidebar {
            width: 320
            height: 480
            annualState: testCase.annualState
            theme: testCase.theme
        }
    }

    function test_ANN_SB_001_sidebarRowsSelectAnnualState() {
        const sidebar = createTemporaryObject(sidebarComponent, testCase)
        const row = TestSupport.findRequired(Lookup, sidebar, "annualSidebarRowMouseArea")
        row.clicked(null)
        compare(annualState.selectedByClick, "annual-1")
    }
}
