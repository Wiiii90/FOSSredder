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

    property var actorViewModel: QtObject {
        property int enterCreateModeCalls: 0
        function enterCreateMode() { enterCreateModeCalls += 1 }
    }

    property var propertyViewModel: QtObject {
        property int enterCreateModeCalls: 0
        function enterCreateMode() { enterCreateModeCalls += 1 }
    }

    property var contractViewModel: QtObject {
        property int enterCreateModeCalls: 0
        function enterCreateMode() { enterCreateModeCalls += 1 }
    }

    property var analysisViewModel: QtObject {
        property string selectedId: "analysis-2"
        function selectAnalysis(id) { selectedId = id }
    }

    property var annualViewModel: QtObject {
        property int resetCreateStateCalls: 0
        function resetCreateState() { resetCreateStateCalls += 1 }
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
            navigation: testCase.navigation
            actorViewModel: testCase.actorViewModel
            propertyViewModel: testCase.propertyViewModel
            contractViewModel: testCase.contractViewModel
            analysisViewModel: testCase.analysisViewModel
            annualViewModel: testCase.annualViewModel
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
        actorViewModel.enterCreateModeCalls = 0
        propertyViewModel.enterCreateModeCalls = 0
        contractViewModel.enterCreateModeCalls = 0
        analysisViewModel.selectedId = "analysis-2"
        annualViewModel.resetCreateStateCalls = 0
    }

    function test_CTRL_TB_001_domainNavigation() {
        const toolbar = createToolbar()

        toolbar.navigateTo(toolbar.navActors, true)
        compare(navigation.sectionValue, navigation.actorSection)
        compare(actorViewModel.enterCreateModeCalls, 1)

        toolbar.navigateTo(toolbar.navProperties, true)
        compare(navigation.sectionValue, navigation.propertySection)
        compare(propertyViewModel.enterCreateModeCalls, 1)

        toolbar.navigateTo(toolbar.navContracts, true)
        compare(navigation.sectionValue, navigation.contractSection)
        compare(contractViewModel.enterCreateModeCalls, 1)
    }

    function test_CTRL_TB_002_bookingAndToolNavigation() {
        const toolbar = createToolbar()

        toolbar.navigateTo(toolbar.navBooking, false)
        compare(navigation.sectionValue, navigation.bookingSection)

        toolbar.navigateTo(toolbar.navAnalysis, true)
        compare(navigation.sectionValue, navigation.analysisSection)
        compare(analysisViewModel.selectedId, "")

        toolbar.navigateTo(toolbar.navAnnual, true)
        compare(navigation.sectionValue, navigation.annualSection)
        compare(annualViewModel.resetCreateStateCalls, 1)
    }

}
