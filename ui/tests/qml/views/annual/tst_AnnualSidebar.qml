/**
 * @file ui/tests/qml/views/annual/tst_AnnualSidebar.qml
 * @brief Provides QML tests for AnnualSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Annual 1.0 as Annual

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnnualSidebarTests"
    when: windowShown
    width: 320
    height: 480

    property var annualViewModel: QtObject {
        property var annualRows: [
            { id: "annual-1", display: "Annual 1", year: 2026 },
            { id: "annual-2", display: "Annual 2", year: 2027 }
        ]
        property string selectedAnnualId: "annual-1"
        property string selectedByClick: ""
        function selectAnnual(id) { selectedByClick = id; selectedAnnualId = id }
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: sidebarComponent
        Annual.AnnualSidebar {
            width: 320
            height: 480
            annualViewModel: testCase.annualViewModel
            theme: testCase.theme
        }
    }

    function test_ANN_SB_001_sidebarRowsSelectAnnualState() {
        const sidebar = createTemporaryObject(sidebarComponent, testCase)
        const row = TestSupport.findRequired(Lookup, sidebar, "annualSidebarRowMouseArea")
        row.clicked(null)
        compare(annualViewModel.selectedByClick, "annual-1")
    }
}
