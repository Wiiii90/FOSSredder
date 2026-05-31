/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisPlotView.qml
 * @brief Provides QML tests for AnalysisPlotView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "AnalysisPlotViewTests"
    when: windowShown
    width: 960
    height: 640

    readonly property string previewImageDataUrl: "data:image/svg+xml,%3Csvg width='1' height='1' viewBox='0 0 1 1'%3E%3Crect width='1' height='1' fill='red'/%3E%3C/svg%3E"
    property var analysisState: QtObject {
        property string renderedPreviewSource: testCase.previewImageDataUrl
    }

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacing: 8
        property int spacingMedium: 10
        property int spacingLarge: 16
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
        property color chartFallback: "#77aadd"
        property var analysis: QtObject {
            property var palette: ["#77aadd", "#88ccaa", "#ddaa77", "#cc88aa"]
            property var layout: QtObject {
                property int splitControlsWidth: 180
            }
            property var table: QtObject {
                property int propertyColumnWidth: 180
                property int amountColumnWidth: 120
                property int totalColumnWidth: 140
                property int rowHeight: 30
            }
        }
        property int chartPlotMinimumHeight: 180
        property int chartPlotPreferredHeight: 260
        property int chartLegendHeight: 80
        property int chartLegendMarkerSize: 12
        property int chartValueLabelWidth: 70
        property int chartPercentLabelWidth: 58
        property string fontFamily: "Segoe UI"
        property int fontSize: 10
    }

    Component {
        id: plotViewComponent
        Analysis.AnalysisPlotView {
            width: 960
            height: 640
            theme: testCase.theme
            analysisState: testCase.analysisState
        }
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function createView() {
        return createTemporaryObject(plotViewComponent, testCase)
    }

    function init() {
        analysisState.renderedPreviewSource = previewImageDataUrl
    }

    function test_ANL_PV_001_renderedArtifactImageTakesOverPreview() {
        const view = createView()
        const image = findRequired(view, "analysisPreviewImage")

        compare(String(image.source), previewImageDataUrl)
        tryCompare(image, "status", Image.Ready)
    }
}
