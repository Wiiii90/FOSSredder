/**
 * @file ui/qml/FossRedder/Components/AppMenu.qml
 * @brief Provides the AppMenu component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15

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

    palette.text: root.theme.appMenuText
    palette.windowText: root.theme.appMenuText
    palette.buttonText: root.theme.appMenuText
    palette.highlight: root.theme.primary
    palette.highlightedText: root.theme.onPrimary
    font.family: root.theme.fontFamily
    font.pointSize: root.theme.appMenuFontSize

    function navigateToSection(section) {
        root.shellNavigationState.navigateToSection(section);
    }

    function navigateToImportHome() {
        root.shellNavigationState.navigateToImportHome();
    }

    background: Rectangle {
        border.width: root.theme.borderWidthThin
        border.color: root.theme.toolbarBorder
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: root.theme.appMenuBackgroundTop
            }
            GradientStop {
                position: 1.0
                color: root.theme.appMenuBackgroundBottom
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: root.theme.shadow
            opacity: root.theme.dark ? 0.24 : 0.07
        }
    }

    Menu {
        title: qsTr("File")
        topPadding: root.theme.appMenuPopupPadding
        bottomPadding: root.theme.appMenuPopupPadding
        leftPadding: 0
        rightPadding: 0
        background: AppMenuPopupFrame { theme: root.theme }
        delegate: AppMenuItem { theme: root.theme }

        Action {
            text: qsTr("New")
            shortcut: StandardKey.New
            onTriggered: if (root.actions)
                root.actions.newFile()
        }

        Action {
            text: qsTr("Open...")
            shortcut: StandardKey.Open
            onTriggered: if (root.actions)
                root.actions.openFile()
        }

        AppMenuSeparator { theme: root.theme }

        Action {
            text: qsTr("Save")
            shortcut: StandardKey.Save
            onTriggered: if (root.actions)
                root.actions.saveFile()
        }

        Action {
            text: qsTr("Save As...")
            shortcut: StandardKey.SaveAs
            onTriggered: if (root.actions)
                root.actions.saveFileAs()
        }

        AppMenuSeparator { theme: root.theme }

        Action {
            text: qsTr("Import...")
            onTriggered: {
                root.navigateToSection(root.navImport);
                if (root.actions)
                    root.actions.browseImportPdf();
            }
        }

        Action {
            text: qsTr("Export...")
            onTriggered: {
                root.navigateToSection(root.navExport);
            }
        }

        AppMenuSeparator { theme: root.theme }

        Action {
            text: qsTr("Quit")
            shortcut: StandardKey.Quit
            onTriggered: {
                if (root.actions && root.actions.quitAction)
                    root.actions.quitAction.trigger();
                else
                    Qt.quit();
            }
        }
    }

    Menu {
        title: qsTr("View")
        topPadding: root.theme.appMenuPopupPadding
        bottomPadding: root.theme.appMenuPopupPadding
        leftPadding: 0
        rightPadding: 0
        background: AppMenuPopupFrame { theme: root.theme }
        delegate: AppMenuItem { theme: root.theme }

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
        AppMenuSeparator { theme: root.theme }
        Action {
            text: qsTr("Analysis")
            onTriggered: root.navigateToSection(root.navAnalysis)
        }
        Action {
            text: qsTr("Annual")
            onTriggered: root.navigateToSection(root.navAnnual)
        }
        AppMenuSeparator { theme: root.theme }
        Action {
            text: qsTr("Settings")
            onTriggered: root.navigateToSection(root.navSettings)
        }
    }

    Menu {
        id: languageMenu
        title: qsTr("Language")
        topPadding: root.theme.appMenuPopupPadding
        bottomPadding: root.theme.appMenuPopupPadding
        leftPadding: 0
        rightPadding: 0
        background: AppMenuPopupFrame { theme: root.theme }
        delegate: AppMenuItem { theme: root.theme }

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
                        return;
                    root.settingsState.selectLanguageAt(index);
                }
            }

            onObjectAdded: function (index, object) {
                languageMenu.insertAction(index, object);
            }
            onObjectRemoved: function (index, object) {
                languageMenu.removeAction(object);
            }
        }
    }

    Menu {
        title: qsTr("Help")
        topPadding: root.theme.appMenuPopupPadding
        bottomPadding: root.theme.appMenuPopupPadding
        leftPadding: 0
        rightPadding: 0
        background: AppMenuPopupFrame { theme: root.theme }
        delegate: AppMenuItem { theme: root.theme }

        Action {
            text: qsTr("About")
            onTriggered: {
                if (root.actions && root.actions.aboutAction)
                    root.actions.aboutAction.trigger();
            }
        }
    }
}
