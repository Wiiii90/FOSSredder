/**
 * @file ui/qml/FossRedder/Components/Layout/SidebarRouter.qml
 * @brief Provides the SidebarRouter component.
 */

import QtQuick 2.15
import FossRedder 1.0 as App
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: sidebarLeft
    required property var appContext
    required property var analysisState
    required property var annualState
    required property var exportState
    required property var settingsState
    required property var theme
    readonly property var navigation: sidebarLeft.appContext ? sidebarLeft.appContext.navigation : null
    readonly property var session: sidebarLeft.appContext ? sidebarLeft.appContext.session : null
    readonly property var workspaceFacade: sidebarLeft.appContext ? sidebarLeft.appContext.workspaceFacade : null
    readonly property var bookingState: sidebarLeft.workspaceFacade ? sidebarLeft.workspaceFacade.bookingState : null
    readonly property int navExport: 5
    readonly property int navAnalysis: 7
    readonly property int navAnnual: 8
    readonly property int navSettings: 6
    readonly property int navStatements: 0

    readonly property int sectionActors: 0
    readonly property int sectionProperties: 1
    readonly property int sectionContracts: 2
    readonly property int sectionBooking: 3
    readonly property int sectionImport: 4

    App.ImportState {
        id: importState
        importWorkflow: sidebarLeft.appContext.importWorkflow
        navigation: sidebarLeft.appContext.navigation
        workspace: sidebarLeft.appContext.workspaceFacade
    }

    property int resolvedSection: -1
    property bool actorLoaded: false
    property bool propertyLoaded: false
    property bool contractLoaded: false
    property bool bookingLoaded: false
    property bool importLoaded: false
    property bool exportLoaded: false
    property bool analysisLoaded: false
    property bool annualLoaded: false
    property bool settingsLoaded: false
    property bool placeholderLoaded: false

    function rememberSection(sec) {
        switch (sec) {
        case sidebarLeft.sectionActors:
            sidebarLeft.actorLoaded = true
            break
        case sidebarLeft.sectionProperties:
            sidebarLeft.propertyLoaded = true
            break
        case sidebarLeft.sectionContracts:
            sidebarLeft.contractLoaded = true
            break
        case sidebarLeft.sectionBooking:
            sidebarLeft.bookingLoaded = true
            break
        case sidebarLeft.sectionImport:
            sidebarLeft.importLoaded = true
            break
        case sidebarLeft.navExport:
            sidebarLeft.exportLoaded = true
            break
        case sidebarLeft.navAnalysis:
            sidebarLeft.analysisLoaded = true
            break
        case sidebarLeft.navAnnual:
            sidebarLeft.annualLoaded = true
            break
        case sidebarLeft.navSettings:
            sidebarLeft.settingsLoaded = true
            break
        default:
            sidebarLeft.placeholderLoaded = true
            break
        }
    }

    function resolveSection() {
        if (!sidebarLeft.navigation) return sidebarLeft.sectionImport;
        var v = sidebarLeft.navigation.sectionValue;
        try {
            if (typeof v === 'string' && v.indexOf('Export') >= 0) return sidebarLeft.navExport;
        } catch(e) {}
        if (typeof v === 'number') return v;
        try {
            var s = String(v);
            if (s.indexOf('Actors') >= 0) return sidebarLeft.sectionActors;
            if (s.indexOf('Properties') >= 0) return sidebarLeft.sectionProperties;
            if (s.indexOf('Contracts') >= 0) return sidebarLeft.sectionContracts;
            if (s.indexOf('Booking') >= 0) return sidebarLeft.sectionBooking;
            if (s.indexOf('Import') >= 0) return sidebarLeft.sectionImport;
            if (s.indexOf('Export') >= 0) return sidebarLeft.navExport;
        } catch (e) {}
        return sidebarLeft.sectionImport;
    }

    function update() {
        var sec = sidebarLeft.resolveSection();

        if (sec !== sidebarLeft.sectionBooking && sidebarLeft.session) {
            if (sidebarLeft.session.selectedTransactionId && sidebarLeft.session.selectedTransactionId.length > 0)
                sidebarLeft.session.selectedTransactionId = ""
            if (sidebarLeft.session.selectedStatementId && sidebarLeft.session.selectedStatementId.length > 0)
                sidebarLeft.session.selectedStatementId = ""
        }

        sidebarLeft.resolvedSection = sec;
        sidebarLeft.rememberSection(sec)
    }

    Component { id: actorSidebarComp; Views.ActorSidebar { appContext: sidebarLeft.appContext; theme: sidebarLeft.theme } }
    Component { id: propertySidebarComp; Views.PropertySidebar { appContext: sidebarLeft.appContext; theme: sidebarLeft.theme } }
    Component { id: contractSidebarComp; Views.ContractSidebar { appContext: sidebarLeft.appContext; theme: sidebarLeft.theme } }
    Component { id: bookingSidebarComp; Views.BookingSidebar { theme: sidebarLeft.theme; bookingState: sidebarLeft.bookingState } }
    Component { id: importSidebarComp; Views.ImportSidebar { importState: importState; theme: sidebarLeft.theme } }
    Component { id: exportSidebarComp; Views.ExportSidebar { exportState: sidebarLeft.exportState; theme: sidebarLeft.theme } }
    Component { id: analysisSidebarComp; Views.AnalysisSidebar { analysisState: sidebarLeft.analysisState; theme: sidebarLeft.theme } }
    Component { id: annualSidebarComp; Views.AnnualSidebar { annualState: sidebarLeft.annualState; theme: sidebarLeft.theme } }
    Component { id: settingsSidebarComp; Views.SettingsSidebar { settingsState: sidebarLeft.settingsState; theme: sidebarLeft.theme } }
    Component { id: placeholderSidebarComp; Views.PlaceholderSidebar { } }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.resolvedSection === sidebarLeft.sectionActors
        contentComponent: sidebarLeft.actorLoaded ? actorSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.resolvedSection === sidebarLeft.sectionProperties
        contentComponent: sidebarLeft.propertyLoaded ? propertySidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.resolvedSection === sidebarLeft.sectionContracts
        contentComponent: sidebarLeft.contractLoaded ? contractSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.resolvedSection === sidebarLeft.sectionBooking
        contentComponent: sidebarLeft.bookingLoaded ? bookingSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.resolvedSection === sidebarLeft.sectionImport
        contentComponent: sidebarLeft.importLoaded ? importSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.resolvedSection === sidebarLeft.navExport
        contentComponent: sidebarLeft.exportLoaded ? exportSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.resolvedSection === sidebarLeft.navAnalysis
        contentComponent: sidebarLeft.analysisLoaded ? analysisSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.resolvedSection === sidebarLeft.navAnnual
        contentComponent: sidebarLeft.annualLoaded ? annualSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.resolvedSection === sidebarLeft.navSettings
        contentComponent: sidebarLeft.settingsLoaded ? settingsSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: sidebarLeft.resolvedSection !== sidebarLeft.sectionActors
                 && sidebarLeft.resolvedSection !== sidebarLeft.sectionProperties
                 && sidebarLeft.resolvedSection !== sidebarLeft.sectionContracts
                 && sidebarLeft.resolvedSection !== sidebarLeft.sectionBooking
                 && sidebarLeft.resolvedSection !== sidebarLeft.sectionImport
                 && sidebarLeft.resolvedSection !== sidebarLeft.navExport
                 && sidebarLeft.resolvedSection !== sidebarLeft.navAnalysis
                 && sidebarLeft.resolvedSection !== sidebarLeft.navAnnual
                 && sidebarLeft.resolvedSection !== sidebarLeft.navSettings
        contentComponent: sidebarLeft.placeholderLoaded ? placeholderSidebarComp : null
    }

    Connections { target: sidebarLeft.navigation; function onSectionChanged() { sidebarLeft.update(); } }

    Component.onCompleted: {
        sidebarLeft.update()
    }
}
