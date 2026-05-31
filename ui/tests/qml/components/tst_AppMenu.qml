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
        function setSectionValue(value) { sectionValue = value }
    }

    property var session: QtObject {
        property string selectedActorId: ""
        property string selectedStatementId: ""
        property string selectedTransactionId: ""
    }

    property var shellNavigationState: QtObject {
        property int actorSection: 0
        property int propertySection: 1
        property int contractSection: 2
        property int bookingSection: 3
        property int importSection: 4
        property int exportSection: 5
        property int settingsSection: 6
        property int analysisSection: 7
        property int annualSection: 8
        property int activeSection: testCase.navigation.sectionValue
        function navigateToSection(section, clearWorkspaceSelection) {
            if (clearWorkspaceSelection) {
                testCase.session.selectedActorId = "";
            } else if (section !== bookingSection) {
                testCase.session.selectedStatementId = "";
                testCase.session.selectedTransactionId = "";
            }
            testCase.navigation.setSectionValue(section);
        }
        function navigateToImportHome() {
            navigateToSection(importSection, false);
        }
        function navigateToBookingCreate() {
            testCase.session.selectedStatementId = "";
            testCase.session.selectedTransactionId = "";
            navigateToSection(bookingSection, false);
        }
    }

    property var settingsState: QtObject {
        property var languageOptions: []
        property string language: ""
        function selectLanguageAt(index) { }
    }

    Component {
        id: appMenuComponent
        AppMenu {
            shellNavigationState: testCase.shellNavigationState
            actions: QtObject {}
            settingsState: testCase.settingsState
            theme: Theme
        }
    }

    function createControl() {
        return createTemporaryObject(appMenuComponent, testCase)
    }

    function init() {
        navigation.sectionValue = -1
        session.selectedActorId = ""
        session.selectedStatementId = ""
        session.selectedTransactionId = ""
    }

    function test_CTRL_AM_001_preservesDomainSelectionAndClearsStaleBookingSelection() {
        const menu = createControl()
        session.selectedActorId = "actor-2"
        session.selectedStatementId = "statement-2"
        session.selectedTransactionId = "tx-2"

        menu.navigateToSection(menu.navActors)
        compare(session.selectedActorId, "actor-2")
        compare(session.selectedStatementId, "")
        compare(session.selectedTransactionId, "")

        session.selectedStatementId = "statement-2"
        session.selectedTransactionId = "tx-2"
        menu.navigateToSection(menu.navBooking)
        compare(session.selectedStatementId, "statement-2")
        compare(session.selectedTransactionId, "tx-2")
    }
}
