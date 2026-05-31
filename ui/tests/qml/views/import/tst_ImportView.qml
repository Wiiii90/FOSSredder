/**
 * @file ui/tests/qml/views/import/tst_ImportView.qml
 * @brief Provides QML composition tests for ImportView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ImportViewTests"
    when: windowShown
    width: 960
    height: 640

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int spacing: 8
        property int spacingSmall: 6
        property int spacingMedium: 8
        property int formLabelWidth: 120
        property int viewActionButtonWidth: 120
        property int viewNavigationButtonWidth: 42
        property int viewCompactActionButtonSize: 28
        property int radius: 3
        property int borderWidthThin: 1
        property int panelPadding: 12
        property int controlHeight: 32
        property color border: "#cccccc"
        property color borderSoft: "#cccccc"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color danger: "#aa0000"
        property color success: "#008800"
        property color successStrong: "#008800"
        property color warning: "#aa8800"
    }

    property var importState: QtObject {
        property var importWorkflow: null
        property var navigation: null
        property var workspace: null
        property int contentIndex: 0
        property var currentDraft: null
        function activatePage() {}
    }

    Component {
        id: importViewComponent
        Import.ImportView {
            width: testCase.width
            height: testCase.height
            importState: testCase.importState
            theme: testCase.theme
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_IMP_V_001_importViewComposesContentStack() {
        const view = createTemporaryObject(importViewComponent, testCase)

        verify(findRequired(view, "importContentStack") !== null)
    }
}
