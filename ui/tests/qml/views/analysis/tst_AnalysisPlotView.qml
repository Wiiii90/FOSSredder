/**
 * @file ui/tests/qml/views/analysis/tst_AnalysisPlotView.qml
 * @brief Provides QML tests for AnalysisPlotView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Analysis 1.0 as Analysis

import "../../common"

TestCase {
    id: testCase
    name: "AnalysisPlotViewTests"
    when: windowShown
    width: 960
    height: 640

    readonly property string previewImageDataUrl: "data:image/svg+xml,%3Csvg width='1' height='1' viewBox='0 0 1 1'%3E%3Crect width='1' height='1' fill='red'/%3E%3C/svg%3E"
    property var analysisViewModel: QtObject {
        property string renderedPreviewSource: testCase.previewImageDataUrl
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: plotViewComponent
        Analysis.AnalysisPlotView {
            width: 960
            height: 640
            theme: testCase.theme
            analysisViewModel: testCase.analysisViewModel
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
        analysisViewModel.renderedPreviewSource = previewImageDataUrl
    }

    function test_ANL_PV_001_renderedArtifactImageTakesOverPreview() {
        const view = createView()
        const image = findRequired(view, "analysisPreviewImage")

        compare(String(image.source), previewImageDataUrl)
        tryCompare(image, "status", Image.Ready)
    }
}
