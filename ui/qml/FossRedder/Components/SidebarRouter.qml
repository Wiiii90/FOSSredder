/**
 * @file ui/qml/FossRedder/Components/SidebarRouter.qml
 * @brief Provides the SidebarRouter component.
 */

import QtQuick 2.15
import FossRedder.Views.Actor 1.0 as Actor
import FossRedder.Views.Analysis 1.0 as Analysis
import FossRedder.Views.Annual 1.0 as Annual
import FossRedder.Views.Booking 1.0 as Booking
import FossRedder.Views.Contract 1.0 as Contract
import FossRedder.Views.Export 1.0 as Export
import FossRedder.Views.Import 1.0 as Import
import FossRedder.Views.Property 1.0 as Property
import FossRedder.Views.Settings 1.0 as Settings
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

    Component { id: actorSidebarComp; Actor.ActorSidebar { actorViewModel: sidebarLeft.actorViewModel; theme: sidebarLeft.theme } }
    Component { id: propertySidebarComp; Property.PropertySidebar { propertyViewModel: sidebarLeft.propertyViewModel; theme: sidebarLeft.theme } }
    Component { id: contractSidebarComp; Contract.ContractSidebar { contractViewModel: sidebarLeft.contractViewModel; theme: sidebarLeft.theme } }
    Component { id: bookingSidebarComp; Booking.BookingSidebar { theme: sidebarLeft.theme; bookingViewModel: sidebarLeft.bookingViewModel } }
    Component { id: importSidebarComp; Import.ImportSidebar { importViewModel: sidebarLeft.importViewModel; theme: sidebarLeft.theme } }
    Component { id: exportSidebarComp; Export.ExportSidebar { exportViewModel: sidebarLeft.exportViewModel; theme: sidebarLeft.theme } }
    Component { id: analysisSidebarComp; Analysis.AnalysisSidebar { analysisViewModel: sidebarLeft.analysisViewModel; theme: sidebarLeft.theme } }
    Component { id: annualSidebarComp; Annual.AnnualSidebar { annualViewModel: sidebarLeft.annualViewModel; theme: sidebarLeft.theme } }
    Component { id: settingsSidebarComp; Settings.SettingsSidebar { settingsViewModel: sidebarLeft.settingsViewModel; theme: sidebarLeft.theme } }
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
