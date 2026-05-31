/**
 * @file ui/tests/qml/components/tst_Toolbar.qml
 * @brief Provides QML tests for Toolbar navigation side effects.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Components 1.0

TestCase {
    id: testCase
    name: "ToolbarTests"
    when: windowShown
    width: 960
    height: 180

    property var navigation: QtObject {
        property int sectionValue: -1
        function setSectionValue(value) { sectionValue = value }
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
                testCase.session.selectedActorId = ""
                testCase.session.selectedPropertyId = ""
                testCase.session.selectedContractId = ""
                testCase.session.selectedStatementId = ""
                testCase.session.selectedTransactionId = ""
                testCase.session.selectedAnalysisId = ""
                testCase.session.selectedAnnualId = ""
            } else if (section !== bookingSection) {
                testCase.session.selectedStatementId = ""
                testCase.session.selectedTransactionId = ""
            }
            testCase.navigation.setSectionValue(section)
        }
        function navigateToImportHome() {
            navigateToSection(importSection, false)
        }
        function navigateToBookingCreate() {
            testCase.session.selectedStatementId = ""
            testCase.session.selectedTransactionId = ""
            navigateToSection(bookingSection, false)
        }
    }

    property var session: QtObject {
        property string selectedActorId: ""
        property string selectedPropertyId: ""
        property string selectedContractId: ""
        property string selectedStatementId: ""
        property string selectedTransactionId: ""
        property string selectedAnalysisId: ""
        property string selectedAnnualId: ""
        function actorRows() { return [{ id: "actor-1" }, { id: "actor-2" }] }
        function propertyRows() { return [{ id: "property-1" }, { id: "property-2" }] }
        function contractRows() { return [{ id: "contract-1" }, { id: "contract-2" }] }
        function statementRows() { return [{ id: "statement-1" }, { id: "statement-2" }] }
        function statementTransactionRows(statementId) {
            return String(statementId || "") === "statement-1" ? [{ id: "tx-1" }] : []
        }
        function analysisRows() { return [{ id: "analysis-1" }, { id: "analysis-2" }] }
        function annualRows() { return [{ id: "annual-1" }, { id: "annual-2" }] }
    }

    property var appContext: QtObject {
        property var navigation: testCase.navigation
        property var session: testCase.session
        property var settingsViewModel: testCase.settingsViewModel
        property var importWorkflow: null
        property var actions: QtObject {}
        property var languageService: QtObject { property var availableLanguages: [] }
    }

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
        property int toolbarHeight: 96
        property int toolbarIconButtonWidth: 64
        property int toolbarSectionSpacing: 2
        property int toolbarGroupSpacing: 4
        property int spacing: 8
        property int spacingSmall: 6
        property int margins: 4
        property int borderWidthThin: 1
        property int radius: 3
        property int fontSizeSmall: 8
        property int toolbarLabelFontSize: 8
        property string fontFamily: "Segoe UI"
        property color toolbarBackground: "#ffffff"
        property color toolbarBorder: "#cccccc"
        property color divider: "#dddddd"
        property color textMuted: "#666666"
        property color textPrimary: "#000000"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color accent: "#3366ff"
    }

    Component {
        id: toolbarComponent
        Toolbar {
            width: 960
            height: 120
            shellNavigationState: testCase.shellNavigationState
            settingsViewModel: testCase.settingsViewModel
            theme: testCase.theme
        }
    }

    function createToolbar() {
        return createTemporaryObject(toolbarComponent, testCase)
    }

    function init() {
        navigation.sectionValue = -1
        session.selectedActorId = ""
        session.selectedPropertyId = ""
        session.selectedContractId = ""
        session.selectedStatementId = ""
        session.selectedTransactionId = ""
        session.selectedAnalysisId = ""
        session.selectedAnnualId = ""
    }

    function test_CTRL_TB_001_domainCreateModeNavigation() {
        const toolbar = createToolbar()
        session.selectedActorId = "actor-2"
        session.selectedPropertyId = "property-2"
        session.selectedContractId = "contract-2"

        toolbar.navigateTo(toolbar.navActors, true)
        compare(session.selectedActorId, "")

        toolbar.navigateTo(toolbar.navProperties, true)
        compare(session.selectedPropertyId, "")

        toolbar.navigateTo(toolbar.navContracts, true)
        compare(session.selectedContractId, "")
    }

    function test_CTRL_TB_002_bookingAndToolCreateModeNavigation() {
        const toolbar = createToolbar()
        session.selectedStatementId = "statement-2"
        session.selectedTransactionId = "tx-2"
        session.selectedAnalysisId = "analysis-2"
        session.selectedAnnualId = "annual-2"

        toolbar.navigateToBookingCreate()
        compare(session.selectedStatementId, "")
        compare(session.selectedTransactionId, "")

        toolbar.navigateTo(toolbar.navAnalysis, true)
        compare(session.selectedAnalysisId, "")

        toolbar.navigateTo(toolbar.navAnnual, true)
        compare(session.selectedAnnualId, "")
    }

}
