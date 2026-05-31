/**
 * @file ui/tests/qml/views/import/tst_StatementDraftView.qml
 * @brief Provides QML composition tests for StatementDraftView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "StatementDraftViewTests"
    when: windowShown
    width: 960
    height: 640

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacingMedium: 10
        property int formLabelWidth: 120
        property int viewActionButtonWidth: 120
        property int viewNavigationButtonWidth: 42
        property int viewCompactActionButtonSize: 28
        property int panelPadding: 12
        property int radius: 3
        property int borderWidthThin: 1
        property int controlHeight: 32
        property color border: "#cccccc"
        property color borderSoft: "#cccccc"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color danger: "#aa0000"
        property color successStrong: "#008800"
        property color warning: "#aa8800"
    }

    property var importState: QtObject {
        property var importWorkflow: null
        property var navigation: null
        property var workspace: null
    }

    Component {
        id: statementViewComponent
        Import.StatementDraftView {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            importState: testCase.importState
            draft: null
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_IMP_D_015_statementDraftViewShowsEmptyStateWithoutDraft() {
        const view = createTemporaryObject(statementViewComponent, null)
        wait(0)

        compare(findRequired(view, "statementDraftState").hasDraft, false)
        compare(findRequired(view, "statementDraftEmptyLabel").visible, true)
    }
}
