/**
 * @file ui/qml/FossRedder/Components/AppMenu.qml
 * @brief Provides the AppMenu component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Effects
import QtQuick.Layouts 1.3

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

    component PopupFrame: Item {
        implicitWidth: Math.max(root.theme.appMenuPopupMinWidth, frame.implicitWidth)
        implicitHeight: frame.implicitHeight

        Rectangle {
            id: shadowSource
            anchors.fill: frame
            radius: frame.radius
            color: root.theme.shadow
            visible: false
        }

        MultiEffect {
            anchors.fill: shadowSource
            source: shadowSource
            shadowEnabled: true
            shadowBlur: 0.34
            shadowColor: root.theme.shadow
            shadowOpacity: root.theme.popupShadowOpacity * 0.55
            shadowVerticalOffset: 3
            shadowHorizontalOffset: 0
        }

        Rectangle {
            id: frame
            anchors.fill: parent
            implicitWidth: root.theme.appMenuPopupMinWidth
            implicitHeight: root.theme.appMenuItemHeight
            radius: root.theme.appMenuPopupRadius
            color: root.theme.appMenuPopupFill
            border.width: root.theme.borderWidthThin
            border.color: root.theme.appMenuPopupBorder
        }
    }

    component StyledMenuItem: MenuItem {
        id: item
        implicitWidth: Math.max(root.theme.appMenuPopupMinWidth, itemContent.implicitWidth + leftPadding + rightPadding)
        implicitHeight: root.theme.appMenuItemHeight
        leftPadding: root.theme.appMenuPopupPadding + root.theme.appMenuItemHorizontalPadding
        rightPadding: root.theme.appMenuPopupPadding + root.theme.appMenuItemHorizontalPadding
        topPadding: 0
        bottomPadding: 0
        font.family: root.theme.fontFamily
        font.pointSize: root.theme.appMenuFontSize

        contentItem: RowLayout {
            id: itemContent
            spacing: root.theme.spacingSmall

            Text {
                text: item.checkable ? (item.checked ? "✓" : "") : ""
                color: item.enabled ? root.theme.appMenuPopupText : root.theme.appMenuPopupMutedText
                font.family: root.theme.fontFamily
                font.pointSize: root.theme.appMenuFontSize
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.preferredWidth: root.theme.appMenuItemCheckWidth
                Layout.fillHeight: true
            }

            Text {
                text: item.text
                color: item.enabled ? root.theme.appMenuPopupText : root.theme.appMenuPopupMutedText
                font.family: root.theme.fontFamily
                font.pointSize: root.theme.appMenuFontSize
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                Layout.fillWidth: true
                Layout.minimumWidth: root.theme.menuItemMinWidth
            }

            Text {
                text: item.action ? String(item.action.shortcut || "") : ""
                color: item.enabled ? root.theme.appMenuPopupMutedText : root.theme.appMenuPopupMutedText
                font.family: root.theme.fontFamily
                font.pointSize: root.theme.appMenuFontSize
                verticalAlignment: Text.AlignVCenter
                visible: text.length > 0
                Layout.leftMargin: root.theme.appMenuItemShortcutSpacing
            }
        }

        background: Rectangle {
            anchors.fill: parent
            anchors.leftMargin: root.theme.appMenuPopupPadding
            anchors.rightMargin: root.theme.appMenuPopupPadding
            radius: root.theme.appMenuItemRadius
            color: item.highlighted ? root.theme.appMenuPopupHoverFill : "transparent"
        }
    }

    component StyledMenuSeparator: MenuSeparator {
        leftPadding: 0
        rightPadding: 0
        topPadding: root.theme.spacingSmall
        bottomPadding: root.theme.spacingSmall

        contentItem: Rectangle {
            implicitHeight: root.theme.borderWidthThin
            color: root.theme.appMenuSeparator
        }
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
        background: PopupFrame {}
        delegate: StyledMenuItem {}

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

        StyledMenuSeparator {}

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

        StyledMenuSeparator {}

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

        StyledMenuSeparator {}

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
        background: PopupFrame {}
        delegate: StyledMenuItem {}

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
        StyledMenuSeparator {}
        Action {
            text: qsTr("Analysis")
            onTriggered: root.navigateToSection(root.navAnalysis)
        }
        Action {
            text: qsTr("Annual")
            onTriggered: root.navigateToSection(root.navAnnual)
        }
        StyledMenuSeparator {}
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
        background: PopupFrame {}
        delegate: StyledMenuItem {}

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
        background: PopupFrame {}
        delegate: StyledMenuItem {}

        Action {
            text: qsTr("About")
            onTriggered: {
                if (root.actions && root.actions.aboutAction)
                    root.actions.aboutAction.trigger();
            }
        }
    }
}
