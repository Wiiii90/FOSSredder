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

    property var settingsState: QtObject {
        property var languageOptions: [
            { code: "en", label: "English", available: true },
            { code: "de", label: "Deutsch", available: true }
        ]
        property int languageIndex: 0
        property string language: "en"
        property int selectedLanguageIndex: -1

        function selectLanguageAt(index) {
            selectedLanguageIndex = index
            const option = languageOptions[index]
            if (!option || option.available === false)
                return
            language = option.code
            languageIndex = index
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
            settingsState: testCase.settingsState
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(settingsGeneralComponent, testCase)
    }

    function init() {
        settingsState.languageOptions = [
            { code: "en", label: "English", available: true },
            { code: "de", label: "Deutsch", available: true }
        ]
        settingsState.language = "en"
        settingsState.languageIndex = 0
        settingsState.selectedLanguageIndex = -1
    }

    function test_SET_G_001_languageSelectionDelegatesToSettingsState() {
        const view = createView()
        const languageDropdown = TestSupport.findRequired(Lookup, view, "settingsLanguageDropdown")

        languageDropdown.currentIndex = 1
        languageDropdown.activated(1)

        compare(settingsState.selectedLanguageIndex, 1)
        compare(settingsState.language, "de")
    }

    function test_SET_G_002_unavailableLanguageSelectionStaysOnCurrentLanguage() {
        settingsState.languageOptions = [
            { code: "en", label: "English", available: true },
            { code: "xx", label: "Unavailable", available: false }
        ]

        const view = createView()
        const languageDropdown = TestSupport.findRequired(Lookup, view, "settingsLanguageDropdown")

        languageDropdown.currentIndex = 1
        languageDropdown.activated(1)

        compare(settingsState.selectedLanguageIndex, 1)
        compare(settingsState.language, "en")
    }

    function test_SET_G_003_languageDropdownReflectsSettingsStateIndex() {
        settingsState.language = "de"
        settingsState.languageIndex = 1

        const view = createView()
        const languageDropdown = TestSupport.findRequired(Lookup, view, "settingsLanguageDropdown")

        compare(languageDropdown.currentIndex, 1)
    }
}
