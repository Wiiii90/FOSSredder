/**
 * @file ui/tests/qml/settings/tst_SettingsMiscellaneous.qml
 * @brief Provides QML tests for SettingsMiscellaneous behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Settings 1.0 as Settings

import "../Lookup.js" as Lookup
import "../TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "SettingsMiscellaneousTests"
    when: windowShown
    width: 960
    height: 640

    property var settingsState: QtObject {
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
            settingsState: testCase.settingsState
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(settingsMiscComponent, testCase)
    }

    function init() {
        settingsState.toolbarShowBooking = true
        settingsState.toolbarShowImport = true
        settingsState.toolbarShowActors = true
        settingsState.toolbarShowExport = true
        settingsState.toolbarShowProperties = true
        settingsState.toolbarShowAnalysis = true
        settingsState.toolbarShowContracts = true
        settingsState.toolbarShowAnnual = true
        settingsState.toolbarShowSettings = true
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

        compare(settingsState.toolbarShowBooking, false)
        compare(settingsState.toolbarShowImport, false)
        compare(settingsState.toolbarShowSettings, false)
    }
}
