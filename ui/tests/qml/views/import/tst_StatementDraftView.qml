/**
 * @file ui/tests/qml/views/import/tst_StatementDraftView.qml
 * @brief Provides QML composition tests for StatementDraftView.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "StatementDraftViewTests"
    when: windowShown
    width: 960
    height: 640

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    property var importViewModel: QtObject {
        property bool hasDraft: false
    }

    Component {
        id: statementViewComponent
        Import.StatementDraftView {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            importViewModel: testCase.importViewModel
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function test_IMP_D_015_statementDraftViewShowsEmptyStateWithoutDraft() {
        const view = createTemporaryObject(statementViewComponent, null)
        wait(0)

        compare(view.importViewModel.hasDraft, false)
        compare(findRequired(view, "statementDraftEmptyLabel").visible, true)
    }
}
