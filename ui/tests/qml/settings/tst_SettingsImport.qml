/**
 * @file ui/tests/qml/settings/tst_SettingsImport.qml
 * @brief Provides QML tests for SettingsImport behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Settings 1.0 as Settings

import "../Lookup.js" as Lookup
import "../TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "SettingsImportTests"
    when: windowShown
    width: 960
    height: 640

    property var settingsState: QtObject {
        property string importDefaultPath: ""
        property string importPoppler: ""
        property string importOpenCv: ""
        property string importTesseract: ""
        property string importParser: ""
        property string importMatcher: ""
        property int browseCalls: 0

        function browseImportPath() { browseCalls += 1 }
    }

    property var theme: QtObject {
        property int spacingSmall: 6
        property int formLabelWidth: 120
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: settingsImportComponent
        Settings.SettingsImport {
            width: 900
            height: 560
            settingsState: testCase.settingsState
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(settingsImportComponent, testCase)
    }

    function init() {
        settingsState.importDefaultPath = ""
        settingsState.importPoppler = ""
        settingsState.importOpenCv = ""
        settingsState.importTesseract = ""
        settingsState.importParser = ""
        settingsState.importMatcher = ""
        settingsState.browseCalls = 0
    }

    function test_SET_I_001_defaultPathFieldUpdatesSettingsState() {
        const view = createView()
        const pathField = TestSupport.findRequired(Lookup, view, "settingsImportDefaultPathField")

        pathField.text = "test:///import/default.pdf"

        compare(settingsState.importDefaultPath, "test:///import/default.pdf")
    }

    function test_SET_I_002_browseButtonDelegatesToSettingsState() {
        const view = createView()
        const browseButton = TestSupport.findRequired(Lookup, view, "settingsImportBrowseButton")

        browseButton.clicked()

        compare(settingsState.browseCalls, 1)
    }

    function test_SET_I_003_pipelineFieldsUpdateSettingsState() {
        const view = createView()
        const fields = [
            { objectName: "settingsImportPopplerField", propertyName: "importPoppler", value: "poppler" },
            { objectName: "settingsImportOpenCvField", propertyName: "importOpenCv", value: "opencv" },
            { objectName: "settingsImportTesseractField", propertyName: "importTesseract", value: "tesseract" },
            { objectName: "settingsImportParserField", propertyName: "importParser", value: "parser" },
            { objectName: "settingsImportMatcherField", propertyName: "importMatcher", value: "matcher" }
        ]

        for (const field of fields) {
            const control = TestSupport.findRequired(Lookup, view, field.objectName)
            control.text = field.value
            compare(settingsState[field.propertyName], field.value)
        }
    }
}
