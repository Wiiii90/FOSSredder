/**
 * @file ui/tests/qml/views/settings/tst_SettingsSidebar.qml
 * @brief Provides QML tests for SettingsSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Settings 1.0 as Settings

import "../../common"

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

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

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

    function test_SET_S_001_categoryRowsDelegateSelectionToSettings() {
        const view = createView()
        const importButton = TestSupport.findRequired(Lookup, view, "settingsSidebarCategoryClick_1")
        wait(0)

        importButton.clicked(null)

        compare(settingsViewModel.selectedCategory, 1)
    }
}
