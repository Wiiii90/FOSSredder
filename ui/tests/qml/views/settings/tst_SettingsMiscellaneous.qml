/**
 * @file ui/tests/qml/views/settings/tst_SettingsMiscellaneous.qml
 * @brief Provides QML tests for SettingsMiscellaneous behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Settings 1.0 as Settings

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "SettingsMiscellaneousTests"
    when: windowShown
    width: 960
    height: 640

    property var settingsViewModel: QtObject {
        property bool toolbarShowBooking: true
        property bool toolbarShowImport: true
        property bool toolbarShowActors: true
        property bool toolbarShowExport: true
        property bool toolbarShowProperties: true
        property bool toolbarShowAnalysis: true
        property bool toolbarShowContracts: true
        property bool toolbarShowAnnual: true
        property bool toolbarShowSettings: true
    }

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacingLarge: 20
        property int formLabelWidth: 120
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: settingsMiscComponent
        Settings.SettingsMiscellaneous {
            width: 900
            height: 560
            settingsViewModel: testCase.settingsViewModel
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(settingsMiscComponent, testCase)
    }

    function init() {
        settingsViewModel.toolbarShowBooking = true
        settingsViewModel.toolbarShowImport = true
        settingsViewModel.toolbarShowActors = true
        settingsViewModel.toolbarShowExport = true
        settingsViewModel.toolbarShowProperties = true
        settingsViewModel.toolbarShowAnalysis = true
        settingsViewModel.toolbarShowContracts = true
        settingsViewModel.toolbarShowAnnual = true
        settingsViewModel.toolbarShowSettings = true
    }

    function test_SET_M_001_toolbarCheckboxesUpdateSettingsState() {
        const view = createView()
        const bookingCheck = TestSupport.findRequired(Lookup, view, "settingsToolbarBookingCheckBox")
        const importCheck = TestSupport.findRequired(Lookup, view, "settingsToolbarImportCheckBox")
        const settingsCheck = TestSupport.findRequired(Lookup, view, "settingsToolbarSettingsCheckBox")

        bookingCheck.checked = false
        bookingCheck.toggled(false)
        importCheck.checked = false
        importCheck.toggled(false)
        settingsCheck.checked = false
        settingsCheck.toggled(false)

        compare(settingsViewModel.toolbarShowBooking, false)
        compare(settingsViewModel.toolbarShowImport, false)
        compare(settingsViewModel.toolbarShowSettings, false)
    }
}
