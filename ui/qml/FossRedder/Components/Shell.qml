/**
 * @file ui/qml/FossRedder/Components/Shell.qml
 * @brief Provides the Shell component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0 as App

GridLayout {
    id: layoutRoot
    required property var appContext
    required property var theme
    anchors.fill: parent
    columns: 1
    columnSpacing: 0
    rowSpacing: 0

    App.AnalysisState {
        id: analysisState
        workspace: layoutRoot.appContext.workspaceFacade
        analysisWorkflow: layoutRoot.appContext.analysisWorkflow
        settingsViewModel: layoutRoot.appContext.settingsViewModel
    }

    App.AnnualState {
        id: annualState
        workspace: layoutRoot.appContext.workspaceFacade
        annualWorkflow: layoutRoot.appContext.annualWorkflow
    }

    App.ExportState {
        id: exportState
        workspace: layoutRoot.appContext.workspaceFacade
        exportWorkflow: layoutRoot.appContext.exportWorkflow
        actions: layoutRoot.appContext.actions
        fileSystemBrowser: layoutRoot.appContext.fileSystemBrowser
        settings: layoutRoot.appContext.settingsViewModel
    }

    App.ImportState {
        id: importState
        importWorkflow: layoutRoot.appContext.importWorkflow
        navigation: layoutRoot.appContext.navigation
        workspace: layoutRoot.appContext.workspaceFacade
        settingsViewModel: layoutRoot.appContext.settingsViewModel
        actions: layoutRoot.appContext.actions
        status: layoutRoot.appContext.status
    }

    App.SettingsState {
        id: settingsState
        navigation: layoutRoot.appContext.navigation
        settingsViewModel: layoutRoot.appContext.settingsViewModel
        actions: layoutRoot.appContext.actions
        languageService: layoutRoot.appContext.languageService
    }

    Binding {
        target: layoutRoot.theme
        property: "mode"
        value: settingsState.themeMode
        restoreMode: Binding.RestoreBinding
    }

    App.ShellNavigationState {
        id: shellNavigationState
        navigation: layoutRoot.appContext.navigation
        workspace: layoutRoot.appContext.workspaceFacade
        importWorkflow: layoutRoot.appContext.importWorkflow
    }

    AppMenu {
        id: appMenu
        Layout.row: 0
        Layout.fillWidth: true
        shellNavigationState: shellNavigationState
        actions: layoutRoot.appContext.actions
        settingsState: settingsState
        theme: layoutRoot.theme
    }

    Toolbar {
        id: toolbar
        Layout.row: 1
        Layout.preferredHeight: toolbar.implicitHeight
        Layout.minimumHeight: toolbar.implicitHeight
        Layout.fillWidth: true
        shellNavigationState: shellNavigationState
        settingsViewModel: layoutRoot.appContext.settingsViewModel
        theme: layoutRoot.theme
    }

    RowLayout {
        id: horizontalLayout
        Layout.row: 2
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: layoutRoot.theme.shellMinimumHeight
        spacing: 0

        Item {
            id: leftWrapper
            Layout.preferredWidth: layoutRoot.theme.shellSidebarPreferredWidth
            Layout.minimumWidth: layoutRoot.theme.shellSidebarMinimumWidth
            Layout.maximumWidth: layoutRoot.theme.shellSidebarPreferredWidth
            Layout.fillHeight: true

            SidebarRouter {
                anchors.fill: parent
                shellNavigationState: shellNavigationState
                actorState: layoutRoot.appContext.workspaceFacade.actorState
                propertyState: layoutRoot.appContext.workspaceFacade.propertyState
                contractState: layoutRoot.appContext.workspaceFacade.contractState
                bookingState: layoutRoot.appContext.workspaceFacade.bookingState
                importState: importState
                analysisState: analysisState
                annualState: annualState
                exportState: exportState
                settingsState: settingsState
                theme: layoutRoot.theme
            }
        }

        Item {
            id: centerWrapper
            Layout.minimumWidth: layoutRoot.theme.shellContentMinimumWidth
            Layout.fillWidth: true
            Layout.fillHeight: true

            ContentRouter {
                anchors.fill: parent
                shellNavigationState: shellNavigationState
                actorState: layoutRoot.appContext.workspaceFacade.actorState
                propertyState: layoutRoot.appContext.workspaceFacade.propertyState
                contractState: layoutRoot.appContext.workspaceFacade.contractState
                bookingState: layoutRoot.appContext.workspaceFacade.bookingState
                importState: importState
                analysisState: analysisState
                annualState: annualState
                exportState: exportState
                settingsState: settingsState
                theme: layoutRoot.theme
            }
        }
    }

    StatusBar {
        id: statusBar
        Layout.row: 3
        Layout.preferredHeight: layoutRoot.theme.statusBarHeight
        Layout.fillWidth: true
        statusState: layoutRoot.appContext.status
        theme: layoutRoot.theme
    }
}
