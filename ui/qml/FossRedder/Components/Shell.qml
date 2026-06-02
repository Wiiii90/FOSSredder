/**
 * @file ui/qml/FossRedder/Components/Shell.qml
 * @brief Provides the Shell component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.3

GridLayout {
    id: layoutRoot
    required property var appContext
    required property var theme
    anchors.fill: parent
    columns: 1
    columnSpacing: 0
    rowSpacing: 0

    readonly property var analysisViewModelObject: layoutRoot.appContext.analysisViewModel
    readonly property var annualViewModelObject: layoutRoot.appContext.annualViewModel
    readonly property var actorViewModelObject: layoutRoot.appContext.actorViewModel
    readonly property var bookingViewModelObject: layoutRoot.appContext.bookingViewModel
    readonly property var contractViewModelObject: layoutRoot.appContext.contractViewModel
    readonly property var exportViewModelObject: layoutRoot.appContext.exportViewModel
    readonly property var importViewModelObject: layoutRoot.appContext.importViewModel
    readonly property var propertyViewModelObject: layoutRoot.appContext.propertyViewModel
    readonly property var settingsViewModelObject: layoutRoot.appContext.settingsViewModel
    readonly property var navigationObject: layoutRoot.appContext.navigation

    Binding {
        target: layoutRoot.theme
        property: "mode"
        value: settingsViewModelObject ? settingsViewModelObject.themeMode : "system"
        restoreMode: Binding.RestoreBinding
    }

    AppMenu {
        id: appMenu
        Layout.row: 0
        Layout.fillWidth: true
        navigation: navigationObject
        actions: layoutRoot.appContext.actions
        settingsViewModel: settingsViewModelObject
        theme: layoutRoot.theme
    }

    Toolbar {
        id: toolbar
        Layout.row: 1
        Layout.preferredHeight: toolbar.implicitHeight
        Layout.minimumHeight: toolbar.implicitHeight
        Layout.fillWidth: true
        navigation: navigationObject
        actorViewModel: actorViewModelObject
        propertyViewModel: propertyViewModelObject
        contractViewModel: contractViewModelObject
        analysisViewModel: analysisViewModelObject
        annualViewModel: annualViewModelObject
        settingsViewModel: settingsViewModelObject
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
                navigation: navigationObject
                actorViewModel: actorViewModelObject
                propertyViewModel: propertyViewModelObject
                contractViewModel: contractViewModelObject
                bookingViewModel: bookingViewModelObject
                importViewModel: importViewModelObject
                analysisViewModel: analysisViewModelObject
                annualViewModel: annualViewModelObject
                exportViewModel: exportViewModelObject
                settingsViewModel: settingsViewModelObject
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
                navigation: navigationObject
                actorViewModel: actorViewModelObject
                propertyViewModel: propertyViewModelObject
                contractViewModel: contractViewModelObject
                bookingViewModel: bookingViewModelObject
                importViewModel: importViewModelObject
                analysisViewModel: analysisViewModelObject
                annualViewModel: annualViewModelObject
                exportViewModel: exportViewModelObject
                settingsViewModel: settingsViewModelObject
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
