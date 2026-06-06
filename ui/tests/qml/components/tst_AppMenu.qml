/**
 * @file ui/tests/qml/components/tst_AppMenu.qml
 * @brief Provides QML tests for AppMenu navigation behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder 1.0
import FossRedder.Components 1.0

TestCase {
    id: testCase
    name: "AppMenuTests"
    when: windowShown
    width: 640
    height: 240

    property var navigation: QtObject {
        property int sectionValue: -1
        property int activeSection: sectionValue
        property int actorSection: 0
        property int propertySection: 1
        property int contractSection: 2
        property int bookingSection: 3
        property int importSection: 4
        property int exportSection: 5
        property int settingsSection: 6
        property int analysisSection: 7
        property int annualSection: 8
        function setSectionValue(value) { sectionValue = value }
        function navigateToSection(section) { sectionValue = section }
    }

    property var settingsViewModel: QtObject {
        property var languageOptions: []
        property string language: ""
        function selectLanguageAt(index) { }
    }

    Component {
        id: appMenuComponent
        AppMenu {
            navigation: testCase.navigation
            actions: QtObject {}
            settingsViewModel: testCase.settingsViewModel
            theme: Theme
        }
    }

    function createControl() {
        return createTemporaryObject(appMenuComponent, testCase)
    }

    function init() {
        navigation.sectionValue = -1
    }

    function test_CTRL_AM_001_navigatesToSections() {
        const menu = createControl()

        menu.navigateToSection(menu.navActors)
        compare(navigation.sectionValue, navigation.actorSection)

        menu.navigateToSection(menu.navBooking)
        compare(navigation.sectionValue, navigation.bookingSection)
    }
}
