/**
 * @file ui/qml/FossRedder/Components/Layout/Shell.qml
 * @brief Provides the Shell component.
 */

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
        workspace: layoutRoot.appContext ? layoutRoot.appContext.workspaceFacade : null
        analysisWorkflow: layoutRoot.appContext ? layoutRoot.appContext.analysisWorkflow : null
        settingsViewModel: layoutRoot.appContext ? layoutRoot.appContext.settingsViewModel : null
    }

    App.AnnualState {
        id: annualState
        workspace: layoutRoot.appContext ? layoutRoot.appContext.workspaceFacade : null
        annualWorkflow: layoutRoot.appContext ? layoutRoot.appContext.annualWorkflow : null
    }

    App.ExportState {
        id: exportState
        workspace: layoutRoot.appContext ? layoutRoot.appContext.workspaceFacade : null
        exportWorkflow: layoutRoot.appContext ? layoutRoot.appContext.exportWorkflow : null
        actions: layoutRoot.appContext ? layoutRoot.appContext.actions : null
        fileSystemBrowser: layoutRoot.appContext ? layoutRoot.appContext.fileSystemBrowser : null
        settings: layoutRoot.appContext ? layoutRoot.appContext.settingsViewModel : null
    }

    App.SettingsState {
        id: settingsState
        navigation: layoutRoot.appContext ? layoutRoot.appContext.navigation : null
        settingsViewModel: layoutRoot.appContext ? layoutRoot.appContext.settingsViewModel : null
        actions: layoutRoot.appContext ? layoutRoot.appContext.actions : null
        languageService: layoutRoot.appContext ? layoutRoot.appContext.languageService : null
    }

    AppMenu {
        id: appMenu
        Layout.row: 0
        Layout.fillWidth: true
        appContext: layoutRoot.appContext
        navigation: layoutRoot.appContext ? layoutRoot.appContext.navigation : null
        actions: layoutRoot.appContext ? layoutRoot.appContext.actions : null
        languageService: layoutRoot.appContext ? layoutRoot.appContext.languageService : null
        settingsViewModel: layoutRoot.appContext ? layoutRoot.appContext.settingsViewModel : null
        theme: layoutRoot.theme
    }

    Toolbar {
        id: toolbar
        Layout.row: 1
        Layout.preferredHeight: toolbar.implicitHeight
        Layout.minimumHeight: toolbar.implicitHeight
        Layout.fillWidth: true
        appContext: layoutRoot.appContext
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
                appContext: layoutRoot.appContext
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
                appContext: layoutRoot.appContext
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
        appContext: layoutRoot.appContext
        theme: layoutRoot.theme
    }

}
