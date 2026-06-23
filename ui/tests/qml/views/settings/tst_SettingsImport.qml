/**
 * @file ui/tests/qml/views/settings/tst_SettingsImport.qml
 * @brief Provides QML tests for SettingsImport behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Settings 1.0 as Settings

import "../../common"

TestCase {
    id: testCase
    name: "SettingsImportTests"
    when: windowShown
    width: 960
    height: 640

    property var settingsViewModel: QtObject {
        property string importDefaultPath: ""
        property string importPoppler: ""
        property string importOpenCv: ""
        property string importTesseract: ""
        property string importParser: ""
        property string importMatcher: ""
        property int browseCalls: 0

        function browseImportPath() { browseCalls += 1 }
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: settingsImportComponent
        Settings.SettingsImport {
            width: 900
            height: 560
            settingsViewModel: testCase.settingsViewModel
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(settingsImportComponent, testCase)
    }

    function init() {
        settingsViewModel.importDefaultPath = ""
        settingsViewModel.importPoppler = ""
        settingsViewModel.importOpenCv = ""
        settingsViewModel.importTesseract = ""
        settingsViewModel.importParser = ""
        settingsViewModel.importMatcher = ""
        settingsViewModel.browseCalls = 0
    }

    function test_SET_I_001_defaultPathFieldUpdatesSettings() {
        const view = createView()
        const pathField = TestSupport.findRequired(Lookup, view, "settingsImportDefaultPathField")

        pathField.text = "test:///import/default.pdf"

        compare(settingsViewModel.importDefaultPath, "test:///import/default.pdf")
    }

    function test_SET_I_002_browseButtonDelegatesToSettings() {
        const view = createView()
        const browseButton = TestSupport.findRequired(Lookup, view, "settingsImportBrowseButton")

        browseButton.clicked()

        compare(settingsViewModel.browseCalls, 1)
    }

    function test_SET_I_003_pipelineFieldsUpdateSettings() {
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
            compare(settingsViewModel[field.propertyName], field.value)
        }
    }
}
