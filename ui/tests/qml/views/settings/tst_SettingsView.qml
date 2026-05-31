/**
 * @file ui/tests/qml/views/settings/tst_SettingsView.qml
 * @brief Provides QML tests for SettingsView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Settings 1.0 as Settings

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "SettingsViewTests"
    when: windowShown
    width: 960
    height: 640

    property var settingsState: QtObject {
        property int currentCategory: 0
        property bool canNavigateCategories: true
        property var languageOptions: [
            { code: "en", label: "English", available: true },
            { code: "de", label: "Deutsch", available: true }
        ]
        property int languageIndex: 0
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
        function selectLanguageAt(index) { languageIndex = index }
        function browseImportPath() {}
        function browseExportDirectory() {}
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int spacingSmall: 6
        property int spacingLarge: 20
        property int formLabelWidth: 120
        property int viewActionButtonWidth: 120
        property int controlHeight: 32
        property int borderWidthThin: 1
        property int radius: 3
        property color subtlePrimaryFill: "#eef3ff"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color borderSoft: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: settingsViewComponent
        Settings.SettingsView {
            width: 960
            height: 640
            settingsState: testCase.settingsState
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(settingsViewComponent, testCase)
    }

    function init() {
        settingsState.currentCategory = 0
        settingsState.activateCalls = 0
        settingsState.saveCalls = 0
        settingsState.resetCalls = 0
    }

    function test_SET_V_001_mountsSettingsStackAndActivatesState() {
        const view = createView()
        const stack = TestSupport.findRequired(Lookup, view, "settingsLoader")

        compare(stack.currentIndex, 0)
        compare(settingsState.activateCalls, 1)
    }

    function test_SET_V_002_categoryNavigationButtonsAdvanceAndReturn() {
        const view = createView()
        const nextButton = TestSupport.findRequired(Lookup, view, "settingsNextCategoryButton")
        const prevButton = TestSupport.findRequired(Lookup, view, "settingsPrevCategoryButton")
        const stack = TestSupport.findRequired(Lookup, view, "settingsLoader")

        nextButton.clicked()
        compare(settingsState.currentCategory, 1)
        compare(stack.currentIndex, 1)

        prevButton.clicked()
        compare(settingsState.currentCategory, 0)
        compare(stack.currentIndex, 0)
    }

    function test_SET_V_003_updateButtonDelegatesToSettingsState() {
        const view = createView()
        TestSupport.findRequired(Lookup, view, "settingsUpdateButton").clicked()

        compare(settingsState.saveCalls, 1)
    }

    function test_SET_V_004_defaultButtonResetsSettingsAndCategory() {
        settingsState.currentCategory = 2
        const view = createView()

        TestSupport.findRequired(Lookup, view, "settingsDefaultButton").clicked()

        compare(settingsState.resetCalls, 1)
        compare(settingsState.currentCategory, 0)
    }

    function test_SET_V_005_categoryNavigationWrapsAtEdges() {
        settingsState.currentCategory = 3
        const view = createView()
        const nextButton = TestSupport.findRequired(Lookup, view, "settingsNextCategoryButton")
        const prevButton = TestSupport.findRequired(Lookup, view, "settingsPrevCategoryButton")

        nextButton.clicked()
        compare(settingsState.currentCategory, 0)

        prevButton.clicked()
        compare(settingsState.currentCategory, 3)
    }
}
