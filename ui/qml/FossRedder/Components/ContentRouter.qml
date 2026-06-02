/**
 * @file ui/qml/FossRedder/Components/ContentRouter.qml
 * @brief Provides the ContentRouter component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: contentRouter
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
    readonly property int navActors: contentRouter.navigation.actorSection
    readonly property int navProperties: contentRouter.navigation.propertySection
    readonly property int navContracts: contentRouter.navigation.contractSection
    readonly property int navBooking: contentRouter.navigation.bookingSection
    readonly property int navImport: contentRouter.navigation.importSection
    readonly property int navExport: contentRouter.navigation.exportSection
    readonly property int navSettings: contentRouter.navigation.settingsSection
    readonly property int navAnalysis: contentRouter.navigation.analysisSection
    readonly property int navAnnual: contentRouter.navigation.annualSection
    Layout.fillWidth: true
    Layout.fillHeight: true

    Component { id: actorViewComp; Views.ActorView { actorViewModel: contentRouter.actorViewModel; theme: contentRouter.theme } }
    Component { id: propertyViewComp; Views.PropertyView { propertyViewModel: contentRouter.propertyViewModel; theme: contentRouter.theme } }
    Component { id: contractViewComp; Views.ContractView { contractViewModel: contentRouter.contractViewModel; theme: contentRouter.theme } }
    Component { id: bookingViewComp; Views.BookingView { bookingViewModel: contentRouter.bookingViewModel; theme: contentRouter.theme } }
    Component { id: importViewComp; Views.ImportView { importViewModel: contentRouter.importViewModel; theme: contentRouter.theme } }
    Component { id: exportViewComp; Views.ExportView { exportViewModel: contentRouter.exportViewModel; theme: contentRouter.theme } }
    Component { id: settingsViewComp; Views.SettingsView { settingsViewModel: contentRouter.settingsViewModel; theme: contentRouter.theme } }
    Component { id: analysisViewComp; Views.AnalysisView { analysisViewModel: contentRouter.analysisViewModel; theme: contentRouter.theme } }
    Component { id: annualViewComp; Views.AnnualView { annualViewModel: contentRouter.annualViewModel; theme: contentRouter.theme } }
    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.navigation.activeSection === contentRouter.navActors
        contentComponent: actorViewComp
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.navigation.activeSection === contentRouter.navProperties
        contentComponent: propertyViewComp
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.navigation.activeSection === contentRouter.navContracts
        contentComponent: contractViewComp
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.navigation.activeSection === contentRouter.navBooking
        contentComponent: bookingViewComp
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.navigation.activeSection === contentRouter.navImport
        contentComponent: importViewComp
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.navigation.activeSection === contentRouter.navExport
        contentComponent: exportViewComp
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.navigation.activeSection === contentRouter.navSettings
        contentComponent: settingsViewComp
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.navigation.activeSection === contentRouter.navAnalysis
        contentComponent: analysisViewComp
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.navigation.activeSection === contentRouter.navAnnual
        contentComponent: annualViewComp
    }
}
