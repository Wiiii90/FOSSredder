/**
 * @file ui/tests/qml/views/settings/tst_SettingsView.qml
 * @brief Provides QML tests for SettingsView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Settings 1.0 as Settings

import "../../common"

TestCase {
    id: testCase
    name: "SettingsViewTests"
    when: windowShown
    width: 960
    height: 640

    property var settingsViewModel: QtObject {
        property int currentCategory: 0
        property bool canNavigateCategories: true
        property var themeModeOptions: [
            { label: "Light", value: "light" },
            { label: "Dark", value: "dark" }
        ]
        property int themeModeIndex: 0
        property var languageOptions: [
            { code: "en", label: "English", available: true },
            { code: "de", label: "Deutsch", available: true },
            { code: "fr", label: "Français", available: true }
        ]
        property int languageIndex: 0
        property bool autosaveOnClose: true
        property var autosaveIntervalOptions: [
            { label: "Off", value: 0 },
            { label: "15 min", value: 15 }
        ]
        property int autosaveIntervalIndex: 0
        property string importDefaultPath: ""
        property string importPoppler: ""
        property string importOpenCv: ""
        property string importTesseract: ""
        property string importParser: ""
        property string importMatcher: ""
        property string exportDefaultDirectory: ""
        property int exportArchiveFormat: 0
        property bool exportIncludeFormulas: true
        property bool toolbarShowBooking: true
        property bool toolbarShowImport: true
        property bool toolbarShowActors: true
        property bool toolbarShowExport: true
        property bool toolbarShowProperties: true
        property bool toolbarShowAnalysis: true
        property bool toolbarShowContracts: true
        property bool toolbarShowAnnual: true
        property bool toolbarShowSettings: true
        property int activateCalls: 0
        property int saveCalls: 0
        property int resetCalls: 0
        function activate() { activateCalls += 1 }
        function navigateCategory(delta) {
            currentCategory = (currentCategory + delta + 4) % 4
        }
        function saveSettings() { saveCalls += 1 }
        function resetSettings() { resetCalls += 1; currentCategory = 0 }
        function selectThemeModeAt(index) { themeModeIndex = index }
        function selectLanguageAt(index) { languageIndex = index }
        function selectAutosaveIntervalAt(index) { autosaveIntervalIndex = index }
        function browseImportPath() {}
        function browseExportDirectory() {}
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: settingsViewComponent
        Settings.SettingsView {
            width: 960
            height: 640
            settingsViewModel: testCase.settingsViewModel
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(settingsViewComponent, testCase)
    }

    function init() {
        settingsViewModel.currentCategory = 0
        settingsViewModel.activateCalls = 0
        settingsViewModel.saveCalls = 0
        settingsViewModel.resetCalls = 0
    }

    function test_SET_V_001_mountsSettingsStackAndActivatesState() {
        const view = createView()
        const stack = TestSupport.findRequired(Lookup, view, "settingsLoader")

        compare(stack.currentIndex, 0)
        compare(settingsViewModel.activateCalls, 1)
    }

    function test_SET_V_002_categoryNavigationButtonsAdvanceAndReturn() {
        const view = createView()
        const nextButton = TestSupport.findRequired(Lookup, view, "settingsNextCategoryButton")
        const prevButton = TestSupport.findRequired(Lookup, view, "settingsPrevCategoryButton")
        const stack = TestSupport.findRequired(Lookup, view, "settingsLoader")

        nextButton.clicked()
        compare(settingsViewModel.currentCategory, 1)
        compare(stack.currentIndex, 1)

        prevButton.clicked()
        compare(settingsViewModel.currentCategory, 0)
        compare(stack.currentIndex, 0)
    }

    function test_SET_V_003_updateButtonDelegatesToSettings() {
        const view = createView()
        TestSupport.findRequired(Lookup, view, "settingsUpdateButton").clicked()

        compare(settingsViewModel.saveCalls, 1)
    }

    function test_SET_V_004_defaultButtonResetsSettingsAndCategory() {
        settingsViewModel.currentCategory = 2
        const view = createView()

        TestSupport.findRequired(Lookup, view, "settingsDefaultButton").clicked()

        compare(settingsViewModel.resetCalls, 1)
        compare(settingsViewModel.currentCategory, 0)
    }

    function test_SET_V_005_categoryNavigationWrapsAtEdges() {
        settingsViewModel.currentCategory = 3
        const view = createView()
        const nextButton = TestSupport.findRequired(Lookup, view, "settingsNextCategoryButton")
        const prevButton = TestSupport.findRequired(Lookup, view, "settingsPrevCategoryButton")

        nextButton.clicked()
        compare(settingsViewModel.currentCategory, 0)

        prevButton.clicked()
        compare(settingsViewModel.currentCategory, 3)
    }
}
