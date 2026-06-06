/**
 * @file ui/tests/qml/views/settings/tst_SettingsGeneral.qml
 * @brief Provides QML tests for SettingsGeneral behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Settings 1.0 as Settings

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "SettingsGeneralTests"
    when: windowShown
    width: 960
    height: 640

    property var settingsViewModel: QtObject {
        property var languageOptions: [
            { code: "en", label: "English", available: true },
            { code: "de", label: "Deutsch", available: true },
            { code: "fr", label: "Français", available: true }
        ]
        property int languageIndex: 0
        property string language: "en"
        property int selectedLanguageIndex: -1
        property var themeModeOptions: [
            { code: "light", label: "Light" },
            { code: "dark", label: "Dark" }
        ]
        property int themeModeIndex: 0
        property string themeMode: "light"
        property int selectedThemeModeIndex: -1
        property bool autosaveOnClose: true
        property var autosaveIntervalOptions: [
            { minutes: 0, label: "Off" },
            { minutes: 5, label: "Every 5 minutes" }
        ]
        property int autosaveIntervalIndex: 0
        property int selectedAutosaveIntervalIndex: -1

        function selectLanguageAt(index) {
            selectedLanguageIndex = index
            const option = languageOptions[index]
            if (!option || option.available === false)
                return
            language = option.code
            languageIndex = index
        }

        function selectThemeModeAt(index) {
            selectedThemeModeIndex = index
            const option = themeModeOptions[index]
            if (!option)
                return
            themeMode = option.code
            themeModeIndex = index
        }

        function selectAutosaveIntervalAt(index) {
            selectedAutosaveIntervalIndex = index
            const option = autosaveIntervalOptions[index]
            if (!option)
                return
            autosaveIntervalIndex = index
        }
    }

    property var theme: QtObject {
        property int spacingSmall: 6
        property int formLabelWidth: 120
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: settingsGeneralComponent
        Settings.SettingsGeneral {
            width: 900
            height: 560
            settingsViewModel: testCase.settingsViewModel
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(settingsGeneralComponent, testCase)
    }

    function init() {
        settingsViewModel.languageOptions = [
            { code: "en", label: "English", available: true },
            { code: "de", label: "Deutsch", available: true },
            { code: "fr", label: "Français", available: true }
        ]
        settingsViewModel.language = "en"
        settingsViewModel.languageIndex = 0
        settingsViewModel.selectedLanguageIndex = -1
        settingsViewModel.themeMode = "light"
        settingsViewModel.themeModeIndex = 0
        settingsViewModel.selectedThemeModeIndex = -1
    }

    function test_SET_G_001_languageSelectionDelegatesToSettings() {
        const view = createView()
        const languageDropdown = TestSupport.findRequired(Lookup, view, "settingsLanguageDropdown")

        languageDropdown.currentIndex = 1
        languageDropdown.activated(1)

        compare(settingsViewModel.selectedLanguageIndex, 1)
        compare(settingsViewModel.language, "de")
    }

    function test_SET_G_002_unavailableLanguageSelectionStaysOnCurrentLanguage() {
        settingsViewModel.languageOptions = [
            { code: "en", label: "English", available: true },
            { code: "xx", label: "Unavailable", available: false }
        ]

        const view = createView()
        const languageDropdown = TestSupport.findRequired(Lookup, view, "settingsLanguageDropdown")

        languageDropdown.currentIndex = 1
        languageDropdown.activated(1)

        compare(settingsViewModel.selectedLanguageIndex, 1)
        compare(settingsViewModel.language, "en")
    }

    function test_SET_G_003_languageDropdownReflectsSettingsIndex() {
        settingsViewModel.language = "de"
        settingsViewModel.languageIndex = 1

        const view = createView()
        const languageDropdown = TestSupport.findRequired(Lookup, view, "settingsLanguageDropdown")

        compare(languageDropdown.currentIndex, 1)
    }

    function test_SET_G_004_themeModeSelectionDelegatesToSettings() {
        const view = createView()
        const themeModeDropdown = TestSupport.findRequired(Lookup, view, "settingsThemeModeDropdown")

        themeModeDropdown.currentIndex = 1
        themeModeDropdown.activated(1)

        compare(settingsViewModel.selectedThemeModeIndex, 1)
        compare(settingsViewModel.themeMode, "dark")
    }

    function test_SET_G_005_autosaveControlsDelegateToSettings() {
        const view = createView()
        const saveOnCloseCheck = TestSupport.findRequired(
                    Lookup, view, "settingsAutosaveOnCloseCheckBox")
        const intervalDropdown = TestSupport.findRequired(
                    Lookup, view, "settingsAutosaveIntervalDropdown")

        saveOnCloseCheck.checked = false
        compare(settingsViewModel.autosaveOnClose, false)

        intervalDropdown.currentIndex = 1
        intervalDropdown.activated(1)
        compare(settingsViewModel.selectedAutosaveIntervalIndex, 1)
        compare(settingsViewModel.autosaveIntervalIndex, 1)
    }
}
