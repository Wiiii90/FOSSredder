/**
 * @file ui/tests/qml/views/settings/tst_SettingsSidebar.qml
 * @brief Provides QML tests for SettingsSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Settings 1.0 as Settings

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "SettingsSidebarTests"
    when: windowShown
    width: 320
    height: 640

    property var settingsViewModel: QtObject {
        property var categoryRows: [
            { category: 0, text: "General", selected: true },
            { category: 1, text: "Import", selected: false },
            { category: 2, text: "Export", selected: false },
            { category: 3, text: "Miscellaneous", selected: false }
        ]
        property int selectedCategory: -1

        function selectCategory(category) { selectedCategory = category }
    }

    property var theme: QtObject {
        property int spacingMedium: 8
        property int spacingSmall: 6
        property int viewSidebarRowHeight: 34
        property int viewSidebarRowRadius: 3
        property int borderWidthThin: 1
        property color selectionHighlight: "#d8e7ff"
        property color borderSoft: "#dddddd"
        property color textPrimary: "#000000"
    }

    Component {
        id: settingsSidebarComponent
        Settings.SettingsSidebar {
            width: 320
            height: 640
            settingsViewModel: testCase.settingsViewModel
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(settingsSidebarComponent, testCase)
    }

    function init() {
        settingsViewModel.selectedCategory = -1
    }

    function test_SET_S_001_categoryRowsDelegateSelectionToSettingsState() {
        const view = createView()
        const importButton = TestSupport.findRequired(Lookup, view, "settingsSidebarCategoryClick_1")
        wait(0)

        importButton.clicked(null)

        compare(settingsViewModel.selectedCategory, 1)
    }
}
