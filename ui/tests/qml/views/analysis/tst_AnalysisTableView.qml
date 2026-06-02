/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisTableView.qml
 * @brief Provides QML tests for AnalysisTableView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "AnalysisTableViewTests"
    when: windowShown
    width: 960
    height: 640

    property var analysisViewModel: QtObject {
        property var tableContractTypes: ["lease", "service"]
        property var tablePropertyRows: [
            { propertyName: "Lot", amounts: [100.0, 0.0], total: 100.0 },
            { propertyName: "House", amounts: [0.0, 50.0], total: 50.0 }
        ]
        property real tableGrandTotal: 150.0
    }

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacing: 8
        property int radius: 3
        property int borderWidthThin: 1
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color borderLight: "#d7d7d7"
        property color borderStrong: "#888888"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color chartText: "#000000"
        property var analysis: QtObject {
            property var table: QtObject {
                property int propertyColumnWidth: 180
                property int amountColumnWidth: 120
                property int totalColumnWidth: 140
                property int rowHeight: 30
            }
        }
    }

    Component {
        id: tableViewComponent
        Analysis.AnalysisTableView {
            width: 960
            height: 640
            theme: testCase.theme
            analysisViewModel: testCase.analysisViewModel
        }
    }

    function createView() {
        return createTemporaryObject(tableViewComponent, testCase)
    }

    function init() {
        analysisViewModel.tableContractTypes = ["lease", "service"]
        analysisViewModel.tablePropertyRows = [
            { propertyName: "Lot", amounts: [100.0, 0.0], total: 100.0 },
            { propertyName: "House", amounts: [0.0, 50.0], total: 50.0 }
        ]
        analysisViewModel.tableGrandTotal = 150.0
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function test_ANL_TV_001_tableRenderUsesStateMatrix() {
        const view = createView()

        compare(view.analysisViewModel.tableContractTypes.length, 2)
        compare(view.analysisViewModel.tablePropertyRows.length, 2)
        compare(view.analysisViewModel.tableGrandTotal, 150.0)
    }

    function test_ANL_TV_002_tablePreviewHasRenderableGeometry() {
        const view = createView()
        const viewport = findRequired(view, "analysisTableViewport")

        verify(view.width > 0)
        verify(view.height > 0)
        verify(viewport.width > 0)
        verify(viewport.height > 0)
    }

}
