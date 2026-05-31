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
    readonly property int navActors: contentRouter.shellNavigationState.actorSection
    readonly property int navProperties: contentRouter.shellNavigationState.propertySection
    readonly property int navContracts: contentRouter.shellNavigationState.contractSection
    readonly property int navBooking: contentRouter.shellNavigationState.bookingSection
    readonly property int navImport: contentRouter.shellNavigationState.importSection
    readonly property int navExport: contentRouter.shellNavigationState.exportSection
    readonly property int navSettings: contentRouter.shellNavigationState.settingsSection
    readonly property int navAnalysis: contentRouter.shellNavigationState.analysisSection
    readonly property int navAnnual: contentRouter.shellNavigationState.annualSection
    Layout.fillWidth: true
    Layout.fillHeight: true

    Component { id: actorViewComp; Views.ActorView { actorState: contentRouter.actorState; theme: contentRouter.theme } }
    Component { id: propertyViewComp; Views.PropertyView { propertyState: contentRouter.propertyState; theme: contentRouter.theme } }
    Component { id: contractViewComp; Views.ContractView { contractState: contentRouter.contractState; theme: contentRouter.theme } }
    Component { id: bookingViewComp; Views.BookingView { bookingState: contentRouter.bookingState; theme: contentRouter.theme } }
    Component { id: importViewComp; Views.ImportView { importState: contentRouter.importState; theme: contentRouter.theme } }
    Component { id: exportViewComp; Views.ExportView { exportState: contentRouter.exportState; theme: contentRouter.theme } }
    Component { id: settingsViewComp; Views.SettingsView { settingsState: contentRouter.settingsState; theme: contentRouter.theme } }
    Component { id: analysisViewComp; Views.AnalysisView { analysisState: contentRouter.analysisState; theme: contentRouter.theme } }
    Component { id: annualViewComp; Views.AnnualView { annualState: contentRouter.annualState; theme: contentRouter.theme } }
    Component { id: placeholderViewComp; Views.PlaceholderView { } }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.shellNavigationState.activeSection === contentRouter.navActors
        contentComponent: contentRouter.shellNavigationState.actorLoaded ? actorViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.shellNavigationState.activeSection === contentRouter.navProperties
        contentComponent: contentRouter.shellNavigationState.propertyLoaded ? propertyViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.shellNavigationState.activeSection === contentRouter.navContracts
        contentComponent: contentRouter.shellNavigationState.contractLoaded ? contractViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.shellNavigationState.activeSection === contentRouter.navBooking
        contentComponent: contentRouter.shellNavigationState.bookingLoaded ? bookingViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.shellNavigationState.activeSection === contentRouter.navImport
        contentComponent: contentRouter.shellNavigationState.importLoaded ? importViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.shellNavigationState.activeSection === contentRouter.navExport
        contentComponent: contentRouter.shellNavigationState.exportLoaded ? exportViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.shellNavigationState.activeSection === contentRouter.navSettings
        contentComponent: contentRouter.shellNavigationState.settingsLoaded ? settingsViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.shellNavigationState.activeSection === contentRouter.navAnalysis
        contentComponent: contentRouter.shellNavigationState.analysisLoaded ? analysisViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.shellNavigationState.activeSection === contentRouter.navAnnual
        contentComponent: contentRouter.shellNavigationState.annualLoaded ? annualViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: !contentRouter.shellNavigationState.isKnownSection(contentRouter.shellNavigationState.activeSection)
        contentComponent: contentRouter.shellNavigationState.placeholderLoaded ? placeholderViewComp : null
    }

    Component.onCompleted: contentRouter.shellNavigationState.activate()
}
