/**
 * @file ui/qml/FossRedder/Components/AppMenu.qml
 * @brief Provides the AppMenu component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
pragma ComponentBehavior: Bound

MenuBar {
    id: root
    required property var shellNavigationState
    required property var actions
    required property var settingsState
    required property var theme
    readonly property int navActors: root.shellNavigationState.actorSection
    readonly property int navProperties: root.shellNavigationState.propertySection
    readonly property int navContracts: root.shellNavigationState.contractSection
    readonly property int navBooking: root.shellNavigationState.bookingSection
    readonly property int navImport: root.shellNavigationState.importSection
    readonly property int navExport: root.shellNavigationState.exportSection
    readonly property int navSettings: root.shellNavigationState.settingsSection
    readonly property int navAnalysis: root.shellNavigationState.analysisSection
    readonly property int navAnnual: root.shellNavigationState.annualSection

    function navigateToSection(section) {
        root.shellNavigationState.navigateToSection(section)
    }

    function navigateToImportHome() {
        root.shellNavigationState.navigateToImportHome()
    }

    background: Rectangle {
        color: root.theme.toolbarBackground
        border.width: root.theme.borderWidthThin
        border.color: root.theme.toolbarBorder
    }

    Menu {
        title: qsTr("File")

        Action {
            text: qsTr("New")
            shortcut: StandardKey.New
            onTriggered: if (root.actions) root.actions.newFile()
        }

        Action {
            text: qsTr("Open...")
            shortcut: StandardKey.Open
            onTriggered: if (root.actions) root.actions.openFile()
        }

        MenuSeparator { }

        Action {
            text: qsTr("Save")
            shortcut: StandardKey.Save
            onTriggered: if (root.actions) root.actions.saveFile()
        }

        Action {
            text: qsTr("Save As...")
            shortcut: StandardKey.SaveAs
            onTriggered: if (root.actions) root.actions.saveFileAs()
        }

        MenuSeparator { }

        Action {
            text: qsTr("Import...")
            onTriggered: {
                root.navigateToSection(root.navImport)
                if (root.actions) root.actions.browseImportPdf()
            }
        }

        Action {
            text: qsTr("Export...")
            onTriggered: {
                root.navigateToSection(root.navExport)
            }
        }

        MenuSeparator { }

        Action {
            text: qsTr("Quit")
            shortcut: StandardKey.Quit
            onTriggered: {
                if (root.actions && root.actions.quitAction) root.actions.quitAction.trigger()
                else Qt.quit()
            }
        }
    }

    Menu {
        title: qsTr("View")

        Action {
            text: qsTr("Import")
            onTriggered: root.navigateToImportHome()
        }
        Action {
            text: qsTr("Export")
            onTriggered: root.navigateToSection(root.navExport)
        }
        Action {
            text: qsTr("Booking")
            onTriggered: root.navigateToSection(root.navBooking)
        }
        Action {
            text: qsTr("Actors")
            onTriggered: root.navigateToSection(root.navActors)
        }
        Action {
            text: qsTr("Properties")
            onTriggered: root.navigateToSection(root.navProperties)
        }
        Action {
            text: qsTr("Contracts")
            onTriggered: root.navigateToSection(root.navContracts)
        }
        MenuSeparator { }
        Action {
            text: qsTr("Analysis")
            onTriggered: root.navigateToSection(root.navAnalysis)
        }
        Action {
            text: qsTr("Annual")
            onTriggered: root.navigateToSection(root.navAnnual)
        }
        MenuSeparator { }
        Action {
            text: qsTr("Settings")
            onTriggered: root.navigateToSection(root.navSettings)
        }
    }

    Menu {
        id: languageMenu
        title: qsTr("Language")

        Instantiator {
            model: root.settingsState.languageOptions

            delegate: Action {
                required property var modelData
                required property int index
                text: modelData.label
                checkable: true
                checked: root.settingsState.language === modelData.code
                enabled: modelData.available !== false
                onTriggered: {
                    if (!enabled)
                        return
                    root.settingsState.selectLanguageAt(index)
                }
            }

            onObjectAdded: function(index, object) { languageMenu.insertAction(index, object) }
            onObjectRemoved: function(index, object) { languageMenu.removeAction(object) }
        }
    }

    Menu {
        title: qsTr("Help")

        Action {
            text: qsTr("About")
            onTriggered: {
                if (root.actions && root.actions.aboutAction) root.actions.aboutAction.trigger()
            }
        }
    }
}
