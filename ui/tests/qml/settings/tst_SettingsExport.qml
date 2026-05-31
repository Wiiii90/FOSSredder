/**
 * @file ui/tests/qml/settings/tst_SettingsExport.qml
 * @brief Provides QML tests for SettingsExport behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Settings 1.0 as Settings

import "../Lookup.js" as Lookup
import "../TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "SettingsExportTests"
    when: windowShown
    width: 960
    height: 640

    property var settingsState: QtObject {
        property string exportDefaultDirectory: ""
        property int exportArchiveFormat: 0
        property bool exportIncludeFormulas: true
        property int browseCalls: 0

        function browseExportDirectory() { browseCalls += 1 }
    }

    property var theme: QtObject {
        property int spacingSmall: 6
        property int formLabelWidth: 120
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: settingsExportComponent
        Settings.SettingsExport {
            width: 900
            height: 560
            settingsState: testCase.settingsState
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(settingsExportComponent, testCase)
    }

    function init() {
        settingsState.exportDefaultDirectory = ""
        settingsState.exportArchiveFormat = 0
        settingsState.exportIncludeFormulas = true
        settingsState.browseCalls = 0
    }

    function test_SET_E_001_defaultDirectoryFieldUpdatesSettingsState() {
        const view = createView()
        const pathField = TestSupport.findRequired(Lookup, view, "settingsExportDefaultDirectoryField")

        pathField.text = "test:///export/out"

        compare(settingsState.exportDefaultDirectory, "test:///export/out")
    }

    function test_SET_E_002_archiveFormatSelectionUpdatesSettingsState() {
        const view = createView()
        const archiveCombo = TestSupport.findRequired(Lookup, view, "settingsExportArchiveFormatComboBox")

        archiveCombo.currentIndex = 1
        archiveCombo.activated(1)

        compare(settingsState.exportArchiveFormat, 1)
    }

    function test_SET_E_003_includeFormulasToggleUpdatesSettingsState() {
        const view = createView()
        const formulasCheck = TestSupport.findRequired(Lookup, view, "settingsExportIncludeFormulasCheckBox")

        formulasCheck.checked = false
        formulasCheck.toggled(false)

        compare(settingsState.exportIncludeFormulas, false)
    }

    function test_SET_E_004_browseButtonDelegatesToSettingsState() {
        const view = createView()
        const browseButton = TestSupport.findRequired(Lookup, view, "settingsExportBrowseButton")

        browseButton.clicked()

        compare(settingsState.browseCalls, 1)
    }
}
