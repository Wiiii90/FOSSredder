/**
 * @file ui/tests/qml/views/settings/tst_SettingsExport.qml
 * @brief Provides QML tests for SettingsExport behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Settings 1.0 as Settings

import "../../common"

TestCase {
    id: testCase
    name: "SettingsExportTests"
    when: windowShown
    width: 960
    height: 640

    property var settingsViewModel: QtObject {
        property string exportDefaultDirectory: ""
        property int exportArchiveFormat: 0
        property bool exportIncludeFormulas: true
        property int browseCalls: 0

        function browseExportDirectory() { browseCalls += 1 }
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: settingsExportComponent
        Settings.SettingsExport {
            width: 900
            height: 560
            settingsViewModel: testCase.settingsViewModel
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(settingsExportComponent, testCase)
    }

    function init() {
        settingsViewModel.exportDefaultDirectory = ""
        settingsViewModel.exportArchiveFormat = 0
        settingsViewModel.exportIncludeFormulas = true
        settingsViewModel.browseCalls = 0
    }

    function test_SET_E_001_defaultDirectoryFieldUpdatesSettings() {
        const view = createView()
        const pathField = TestSupport.findRequired(Lookup, view, "settingsExportDefaultDirectoryField")

        pathField.text = "test:///export/out"

        compare(settingsViewModel.exportDefaultDirectory, "test:///export/out")
    }

    function test_SET_E_002_archiveFormatSelectionUpdatesSettings() {
        const view = createView()
        const archiveCombo = TestSupport.findRequired(Lookup, view, "settingsExportArchiveFormatComboBox")

        archiveCombo.currentIndex = 1
        archiveCombo.activated(1)

        compare(settingsViewModel.exportArchiveFormat, 1)
    }

    function test_SET_E_003_includeFormulasToggleUpdatesSettings() {
        const view = createView()
        const formulasCheck = TestSupport.findRequired(Lookup, view, "settingsExportIncludeFormulasCheckBox")

        formulasCheck.checked = false
        formulasCheck.toggled()

        compare(settingsViewModel.exportIncludeFormulas, false)
    }

    function test_SET_E_004_browseButtonDelegatesToSettings() {
        const view = createView()
        const browseButton = TestSupport.findRequired(Lookup, view, "settingsExportBrowseButton")

        browseButton.clicked()

        compare(settingsViewModel.browseCalls, 1)
    }
}
