/**
 * @file ui/qml/FossRedder/Components/SidebarRouter.qml
 * @brief Provides the SidebarRouter component.
 */

import QtQuick 2.15
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: sidebarLeft
    required property var navigation
    required property var actorViewModel
    required property var propertyViewModel
    required property var contractViewModel
    required property var bookingViewModel
    required property var importViewModel
    required property var analysisViewModel
    required property var annualViewModel
    required property var exportViewModel
    required property var settingsViewModel
    required property var theme
    readonly property int sectionActors: sidebarLeft.navigation.actorSection
    readonly property int sectionProperties: sidebarLeft.navigation.propertySection
    readonly property int sectionContracts: sidebarLeft.navigation.contractSection
    readonly property int sectionBooking: sidebarLeft.navigation.bookingSection
    readonly property int sectionImport: sidebarLeft.navigation.importSection
    readonly property int sectionExport: sidebarLeft.navigation.exportSection
    readonly property int sectionAnalysis: sidebarLeft.navigation.analysisSection
    readonly property int sectionAnnual: sidebarLeft.navigation.annualSection
    readonly property int sectionSettings: sidebarLeft.navigation.settingsSection

    Component { id: actorSidebarComp; Views.ActorSidebar { actorViewModel: sidebarLeft.actorViewModel; theme: sidebarLeft.theme } }
    Component { id: propertySidebarComp; Views.PropertySidebar { propertyViewModel: sidebarLeft.propertyViewModel; theme: sidebarLeft.theme } }
    Component { id: contractSidebarComp; Views.ContractSidebar { contractViewModel: sidebarLeft.contractViewModel; theme: sidebarLeft.theme } }
    Component { id: bookingSidebarComp; Views.BookingSidebar { theme: sidebarLeft.theme; bookingViewModel: sidebarLeft.bookingViewModel } }
    Component { id: importSidebarComp; Views.ImportSidebar { importViewModel: sidebarLeft.importViewModel; theme: sidebarLeft.theme } }
    Component { id: exportSidebarComp; Views.ExportSidebar { exportViewModel: sidebarLeft.exportViewModel; theme: sidebarLeft.theme } }
    Component { id: analysisSidebarComp; Views.AnalysisSidebar { analysisViewModel: sidebarLeft.analysisViewModel; theme: sidebarLeft.theme } }
    Component { id: annualSidebarComp; Views.AnnualSidebar { annualViewModel: sidebarLeft.annualViewModel; theme: sidebarLeft.theme } }
    Component { id: settingsSidebarComp; Views.SettingsSidebar { settingsViewModel: sidebarLeft.settingsViewModel; theme: sidebarLeft.theme } }
    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.navigation.activeSection === sidebarLeft.sectionActors
        contentComponent: actorSidebarComp
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.navigation.activeSection === sidebarLeft.sectionProperties
        contentComponent: propertySidebarComp
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.navigation.activeSection === sidebarLeft.sectionContracts
        contentComponent: contractSidebarComp
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.navigation.activeSection === sidebarLeft.sectionBooking
        contentComponent: bookingSidebarComp
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.navigation.activeSection === sidebarLeft.sectionImport
        contentComponent: importSidebarComp
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.navigation.activeSection === sidebarLeft.sectionExport
        contentComponent: exportSidebarComp
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.navigation.activeSection === sidebarLeft.sectionAnalysis
        contentComponent: analysisSidebarComp
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.navigation.activeSection === sidebarLeft.sectionAnnual
        contentComponent: annualSidebarComp
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.navigation.activeSection === sidebarLeft.sectionSettings
        contentComponent: settingsSidebarComp
    }
}
