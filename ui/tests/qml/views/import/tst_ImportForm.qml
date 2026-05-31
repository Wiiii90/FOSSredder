/**
 * @file ui/tests/qml/views/import/tst_ImportForm.qml
 * @brief Provides QML tests for ImportForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ImportFormTests"
    when: windowShown
    width: 960
    height: 640

    property var theme: QtObject {
        property int spacingSmall: 6
        property int formLabelWidth: 120
    }

    property var importState: QtObject {
        property var importSourceLabels: ["PDF"]
        property var statementStrategyLabels: ["Commerzbank26"]
    }

    Component {
        id: importFormComponent

        Import.ImportForm {
            width: 600
            theme: testCase.theme
            importState: testCase.importState
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createForm() {
        return createTemporaryObject(importFormComponent, testCase)
    }

    function test_IMP_F_001_sourceSelectorShowsSupportedLabels() {
        const form = createForm()
        const sourceCombo = findRequired(form, "importSourceComboBox")

        verify(sourceCombo.model !== null)
        compare(sourceCombo.model.length, 1)
        compare(sourceCombo.model[0], "PDF")
    }

    function test_IMP_F_002_strategySelectorShowsSupportedLabels() {
        const form = createForm()
        const strategyCombo = findRequired(form, "importStrategyComboBox")

        verify(strategyCombo.model !== null)
        compare(strategyCombo.model.length, 1)
        compare(strategyCombo.model[0], "Commerzbank26")
    }

}
