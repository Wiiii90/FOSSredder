/**
 * @file ui/qml/FossRedder/Components/SidebarRouter.qml
 * @brief Provides the SidebarRouter component.
 */

import QtQuick 2.15
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: sidebarLeft
    required property var shellNavigationState
    required property var actorState
    required property var propertyState
    required property var contractState
    required property var bookingState
    required property var importState
    required property var analysisState
    required property var annualState
    required property var exportState
    required property var settingsState
    required property var theme
    readonly property int sectionActors: sidebarLeft.shellNavigationState.actorSection
    readonly property int sectionProperties: sidebarLeft.shellNavigationState.propertySection
    readonly property int sectionContracts: sidebarLeft.shellNavigationState.contractSection
    readonly property int sectionBooking: sidebarLeft.shellNavigationState.bookingSection
    readonly property int sectionImport: sidebarLeft.shellNavigationState.importSection
    readonly property int sectionExport: sidebarLeft.shellNavigationState.exportSection
    readonly property int sectionAnalysis: sidebarLeft.shellNavigationState.analysisSection
    readonly property int sectionAnnual: sidebarLeft.shellNavigationState.annualSection
    readonly property int sectionSettings: sidebarLeft.shellNavigationState.settingsSection

    Component { id: actorSidebarComp; Views.ActorSidebar { actorState: sidebarLeft.actorState; theme: sidebarLeft.theme } }
    Component { id: propertySidebarComp; Views.PropertySidebar { propertyState: sidebarLeft.propertyState; theme: sidebarLeft.theme } }
    Component { id: contractSidebarComp; Views.ContractSidebar { contractState: sidebarLeft.contractState; theme: sidebarLeft.theme } }
    Component { id: bookingSidebarComp; Views.BookingSidebar { theme: sidebarLeft.theme; bookingState: sidebarLeft.bookingState } }
    Component { id: importSidebarComp; Views.ImportSidebar { importState: sidebarLeft.importState; theme: sidebarLeft.theme } }
    Component { id: exportSidebarComp; Views.ExportSidebar { exportState: sidebarLeft.exportState; theme: sidebarLeft.theme } }
    Component { id: analysisSidebarComp; Views.AnalysisSidebar { analysisState: sidebarLeft.analysisState; theme: sidebarLeft.theme } }
    Component { id: annualSidebarComp; Views.AnnualSidebar { annualState: sidebarLeft.annualState; theme: sidebarLeft.theme } }
    Component { id: settingsSidebarComp; Views.SettingsSidebar { settingsState: sidebarLeft.settingsState; theme: sidebarLeft.theme } }
    Component { id: placeholderSidebarComp; Views.PlaceholderSidebar { } }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.shellNavigationState.activeSection === sidebarLeft.sectionActors
        contentComponent: sidebarLeft.shellNavigationState.actorLoaded ? actorSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.shellNavigationState.activeSection === sidebarLeft.sectionProperties
        contentComponent: sidebarLeft.shellNavigationState.propertyLoaded ? propertySidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.shellNavigationState.activeSection === sidebarLeft.sectionContracts
        contentComponent: sidebarLeft.shellNavigationState.contractLoaded ? contractSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.shellNavigationState.activeSection === sidebarLeft.sectionBooking
        contentComponent: sidebarLeft.shellNavigationState.bookingLoaded ? bookingSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.shellNavigationState.activeSection === sidebarLeft.sectionImport
        contentComponent: sidebarLeft.shellNavigationState.importLoaded ? importSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.shellNavigationState.activeSection === sidebarLeft.sectionExport
        contentComponent: sidebarLeft.shellNavigationState.exportLoaded ? exportSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.shellNavigationState.activeSection === sidebarLeft.sectionAnalysis
        contentComponent: sidebarLeft.shellNavigationState.analysisLoaded ? analysisSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.shellNavigationState.activeSection === sidebarLeft.sectionAnnual
        contentComponent: sidebarLeft.shellNavigationState.annualLoaded ? annualSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.shellNavigationState.activeSection === sidebarLeft.sectionSettings
        contentComponent: sidebarLeft.shellNavigationState.settingsLoaded ? settingsSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: !sidebarLeft.shellNavigationState.isKnownSection(sidebarLeft.shellNavigationState.activeSection)
        contentComponent: sidebarLeft.shellNavigationState.placeholderLoaded ? placeholderSidebarComp : null
    }

    Component.onCompleted: sidebarLeft.shellNavigationState.activate()
}
