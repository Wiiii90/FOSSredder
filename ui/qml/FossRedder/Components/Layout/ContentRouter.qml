/**
 * @file ui/qml/FossRedder/Components/Layout/ContentRouter.qml
 * @brief Provides the ContentRouter component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: contentRouter
    required property var appContext
    required property var analysisState
    required property var annualState
    required property var exportState
    required property var settingsState
    required property var theme
    readonly property var navigation: contentRouter.appContext ? contentRouter.appContext.navigation : null
    readonly property var session: contentRouter.appContext ? contentRouter.appContext.session : null
    readonly property int navActors: 0
    readonly property int navProperties: 1
    readonly property int navContracts: 2
    readonly property int navBooking: 3
    readonly property int navImport: 4
    readonly property int navExport: 5
    readonly property int navSettings: 6
    readonly property int navAnalysis: 7
    readonly property int navAnnual: 8
    readonly property int navBookingStatements: 0
    Layout.fillWidth: true
    Layout.fillHeight: true
    property int activeSection: contentRouter.navigation ? contentRouter.navigation.sectionValue : contentRouter.navImport
    property bool actorLoaded: false
    property bool propertyLoaded: false
    property bool contractLoaded: false
    property bool bookingLoaded: false
    property bool importLoaded: false
    property bool exportLoaded: false
    property bool settingsLoaded: false
    property bool analysisLoaded: false
    property bool annualLoaded: false
    property bool placeholderLoaded: false

    Component { id: actorViewComp; Views.ActorView { appContext: contentRouter.appContext; theme: contentRouter.theme } }
    Component { id: propertyViewComp; Views.PropertyView { appContext: contentRouter.appContext; theme: contentRouter.theme } }
    Component { id: contractViewComp; Views.ContractView { appContext: contentRouter.appContext; theme: contentRouter.theme } }
    Component { id: bookingViewComp; Views.BookingView { appContext: contentRouter.appContext; theme: contentRouter.theme } }
    Component { id: importViewComp; Views.ImportView { appContext: contentRouter.appContext; theme: contentRouter.theme } }
    Component { id: exportViewComp; Views.ExportView { exportState: contentRouter.exportState; theme: contentRouter.theme } }
    Component { id: settingsViewComp; Views.SettingsView { settingsState: contentRouter.settingsState; theme: contentRouter.theme } }
    Component { id: analysisViewComp; Views.AnalysisView { appContext: contentRouter.appContext; analysisState: contentRouter.analysisState; theme: contentRouter.theme } }
    Component { id: annualViewComp; Views.AnnualView { annualState: contentRouter.annualState; theme: contentRouter.theme } }
    Component { id: placeholderViewComp; Views.PlaceholderView { } }

    function rememberSection(section) {
        switch (section) {
        case contentRouter.navActors:
            contentRouter.actorLoaded = true
            break
        case contentRouter.navProperties:
            contentRouter.propertyLoaded = true
            break
        case contentRouter.navContracts:
            contentRouter.contractLoaded = true
            break
        case contentRouter.navBooking:
            contentRouter.bookingLoaded = true
            break
        case contentRouter.navImport:
            contentRouter.importLoaded = true
            break
        case contentRouter.navExport:
            contentRouter.exportLoaded = true
            break
        case contentRouter.navSettings:
            contentRouter.settingsLoaded = true
            break
        case contentRouter.navAnalysis:
            contentRouter.analysisLoaded = true
            break
        case contentRouter.navAnnual:
            contentRouter.annualLoaded = true
            break
        default:
            contentRouter.placeholderLoaded = true
            break
        }
    }

    function updateContent() {
        if (contentRouter.navigation && contentRouter.navigation.sectionValue !== contentRouter.navBooking && contentRouter.session) {
            if (contentRouter.session.selectedTransactionId && contentRouter.session.selectedTransactionId.length > 0)
                contentRouter.session.selectedTransactionId = ""
            if (contentRouter.session.selectedStatementId && contentRouter.session.selectedStatementId.length > 0)
                contentRouter.session.selectedStatementId = ""
        }

        contentRouter.activeSection = contentRouter.navigation ? contentRouter.navigation.sectionValue : contentRouter.navImport
        contentRouter.rememberSection(contentRouter.activeSection)
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.activeSection === contentRouter.navActors
        contentComponent: contentRouter.actorLoaded ? actorViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.activeSection === contentRouter.navProperties
        contentComponent: contentRouter.propertyLoaded ? propertyViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.activeSection === contentRouter.navContracts
        contentComponent: contentRouter.contractLoaded ? contractViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.activeSection === contentRouter.navBooking
        contentComponent: contentRouter.bookingLoaded ? bookingViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.activeSection === contentRouter.navImport
        contentComponent: contentRouter.importLoaded ? importViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.activeSection === contentRouter.navExport
        contentComponent: contentRouter.exportLoaded ? exportViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.activeSection === contentRouter.navSettings
        contentComponent: contentRouter.settingsLoaded ? settingsViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.activeSection === contentRouter.navAnalysis
        contentComponent: contentRouter.analysisLoaded ? analysisViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.activeSection === contentRouter.navAnnual
        contentComponent: contentRouter.annualLoaded ? annualViewComp : null
    }

    Content {
        anchors.fill: parent
        theme: contentRouter.theme
        visible: contentRouter.activeSection !== contentRouter.navActors
                 && contentRouter.activeSection !== contentRouter.navProperties
                 && contentRouter.activeSection !== contentRouter.navContracts
                 && contentRouter.activeSection !== contentRouter.navBooking
                 && contentRouter.activeSection !== contentRouter.navImport
                 && contentRouter.activeSection !== contentRouter.navExport
                 && contentRouter.activeSection !== contentRouter.navSettings
                 && contentRouter.activeSection !== contentRouter.navAnalysis
                 && contentRouter.activeSection !== contentRouter.navAnnual
        contentComponent: contentRouter.placeholderLoaded ? placeholderViewComp : null
    }

    Component.onCompleted: contentRouter.updateContent()
    Connections {
        target: contentRouter.navigation
        function onSectionChanged() {
            contentRouter.updateContent()
        }
    }
}
