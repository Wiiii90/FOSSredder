/**
 * @file ui/qml/FossRedder/Components/Toolbar.qml
 * @brief Provides the Toolbar component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: toolBar
    required property var navigation
    required property var actorViewModel
    required property var propertyViewModel
    required property var contractViewModel
    required property var analysisViewModel
    required property var annualViewModel
    required property var settingsViewModel
    required property var theme
    readonly property int navActors: toolBar.navigation.actorSection
    readonly property int navProperties: toolBar.navigation.propertySection
    readonly property int navContracts: toolBar.navigation.contractSection
    readonly property int navBooking: toolBar.navigation.bookingSection
    readonly property int navImport: toolBar.navigation.importSection
    readonly property int navExport: toolBar.navigation.exportSection
    readonly property int navSettings: toolBar.navigation.settingsSection
    readonly property int navAnalysis: toolBar.navigation.analysisSection
    readonly property int navAnnual: toolBar.navigation.annualSection
    readonly property bool fileGroupActive: toolBar.navigation.activeSection === toolBar.navImport || toolBar.navigation.activeSection === toolBar.navExport
    readonly property bool domainGroupActive: toolBar.navigation.activeSection === toolBar.navBooking || toolBar.navigation.activeSection === toolBar.navActors || toolBar.navigation.activeSection === toolBar.navProperties || toolBar.navigation.activeSection === toolBar.navContracts
    readonly property bool toolsGroupActive: toolBar.navigation.activeSection === toolBar.navAnalysis || toolBar.navigation.activeSection === toolBar.navAnnual
    readonly property bool appGroupActive: toolBar.navigation.activeSection === toolBar.navSettings
    implicitHeight: toolBar.theme.toolbarHeight
    property int iconRowHeight: toolBar.theme.toolbarIconRowHeight

    function assetUrl(fileName) {
        return Qt.resolvedUrl("../assets/" + fileName);
    }

    function prepareCreateMode(section) {
        if (section === toolBar.navActors && toolBar.actorViewModel) {
            toolBar.actorViewModel.enterCreateMode();
        } else if (section === toolBar.navProperties && toolBar.propertyViewModel) {
            toolBar.propertyViewModel.enterCreateMode();
        } else if (section === toolBar.navContracts && toolBar.contractViewModel) {
            toolBar.contractViewModel.enterCreateMode();
        } else if (section === toolBar.navAnalysis && toolBar.analysisViewModel) {
            toolBar.analysisViewModel.selectAnalysis("");
        } else if (section === toolBar.navAnnual && toolBar.annualViewModel) {
            toolBar.annualViewModel.resetCreateState();
        }
    }

    function navigateTo(section, createMode) {
        if (createMode)
            toolBar.prepareCreateMode(section);
        toolBar.navigation.navigateToSection(section);
    }

    function showDividerForGroup(visibleGroup, hasVisibleAfter) {
        return visibleGroup && hasVisibleAfter;
    }

    Rectangle {
        id: bg
        anchors.left: parent.left
        anchors.right: parent.right
        height: toolBar.implicitHeight
        border.width: toolBar.theme.borderWidthThin
        border.color: toolBar.theme.toolbarBorder
        clip: false
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: toolBar.theme.toolbarBackgroundTop
            }
            GradientStop {
                position: 1.0
                color: toolBar.theme.toolbarBackgroundBottom
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: toolBar.theme.shadow
            opacity: toolBar.theme.dark ? 0.28 : 0.08
        }

        RowLayout {
            id: rootRow
            anchors.fill: parent
            anchors.leftMargin: toolBar.theme.toolbarGroupSpacing
            anchors.rightMargin: toolBar.theme.toolbarGroupSpacing
            spacing: toolBar.theme.spacing + toolBar.theme.margins
            Layout.alignment: Qt.AlignVCenter

            ToolbarGroup {
                id: fileGroup
                visible: toolBar.settingsViewModel.toolbarShowImport || toolBar.settingsViewModel.toolbarShowExport
                theme: toolBar.theme
                label: qsTr("File")
                active: toolBar.fileGroupActive
                iconRowHeight: toolBar.iconRowHeight

                Controls.IconButton {
                    visible: toolBar.settingsViewModel.toolbarShowImport
                    Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                    Layout.preferredHeight: fileGroup.actionHeight
                    svgSource: toolBar.assetUrl("import.svg")
                    label: qsTr("Import")
                    active: toolBar.navigation.activeSection === toolBar.navImport
                    onClicked: toolBar.navigateTo(toolBar.navImport, false)
                }
                Controls.IconButton {
                    visible: toolBar.settingsViewModel.toolbarShowExport
                    Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                    Layout.preferredHeight: fileGroup.actionHeight
                    svgSource: toolBar.assetUrl("export.svg")
                    label: qsTr("Export")
                    active: toolBar.navigation.activeSection === toolBar.navExport
                    onClicked: toolBar.navigateTo(toolBar.navExport, false)
                }
            }

            ToolbarDivider {
                theme: toolBar.theme
                visible: toolBar.showDividerForGroup(fileGroup.visible, domainGroup.visible || toolsGroup.visible || appGroup.visible)
            }

            ToolbarGroup {
                id: domainGroup
                visible: toolBar.settingsViewModel.toolbarShowBooking || toolBar.settingsViewModel.toolbarShowActors || toolBar.settingsViewModel.toolbarShowProperties || toolBar.settingsViewModel.toolbarShowContracts
                theme: toolBar.theme
                label: qsTr("Domain")
                active: toolBar.domainGroupActive
                iconRowHeight: toolBar.iconRowHeight

                Controls.IconButton {
                    visible: toolBar.settingsViewModel.toolbarShowBooking
                    Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                    Layout.preferredHeight: domainGroup.actionHeight
                    svgSource: toolBar.assetUrl("booking.svg")
                    label: qsTr("Booking")
                    active: toolBar.navigation.activeSection === toolBar.navBooking
                    onClicked: toolBar.navigateTo(toolBar.navBooking, false)
                }
                Controls.IconButton {
                    visible: toolBar.settingsViewModel.toolbarShowActors
                    Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                    Layout.preferredHeight: domainGroup.actionHeight
                    svgSource: toolBar.assetUrl("actor.svg")
                    label: qsTr("Actor")
                    active: toolBar.navigation.activeSection === toolBar.navActors
                    onClicked: toolBar.navigateTo(toolBar.navActors, true)
                }
                Controls.IconButton {
                    visible: toolBar.settingsViewModel.toolbarShowProperties
                    Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                    Layout.preferredHeight: domainGroup.actionHeight
                    svgSource: toolBar.assetUrl("property.svg")
                    label: qsTr("Property")
                    active: toolBar.navigation.activeSection === toolBar.navProperties
                    onClicked: toolBar.navigateTo(toolBar.navProperties, true)
                }
                Controls.IconButton {
                    visible: toolBar.settingsViewModel.toolbarShowContracts
                    Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                    Layout.preferredHeight: domainGroup.actionHeight
                    svgSource: toolBar.assetUrl("contract.svg")
                    label: qsTr("Contract")
                    active: toolBar.navigation.activeSection === toolBar.navContracts
                    onClicked: toolBar.navigateTo(toolBar.navContracts, true)
                }
            }

            ToolbarDivider {
                theme: toolBar.theme
                visible: toolBar.showDividerForGroup(domainGroup.visible, toolsGroup.visible || appGroup.visible)
            }

            ToolbarGroup {
                id: toolsGroup
                visible: toolBar.settingsViewModel.toolbarShowAnalysis || toolBar.settingsViewModel.toolbarShowAnnual
                theme: toolBar.theme
                label: qsTr("Tools")
                active: toolBar.toolsGroupActive
                iconRowHeight: toolBar.iconRowHeight

                Controls.IconButton {
                    visible: toolBar.settingsViewModel.toolbarShowAnalysis
                    Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                    Layout.preferredHeight: toolsGroup.actionHeight
                    svgSource: toolBar.assetUrl("analysis.svg")
                    label: qsTr("Analysis")
                    active: toolBar.navigation.activeSection === toolBar.navAnalysis
                    onClicked: toolBar.navigateTo(toolBar.navAnalysis, true)
                }
                Controls.IconButton {
                    visible: toolBar.settingsViewModel.toolbarShowAnnual
                    Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                    Layout.preferredHeight: toolsGroup.actionHeight
                    svgSource: toolBar.assetUrl("annual.svg")
                    label: qsTr("Annual")
                    active: toolBar.navigation.activeSection === toolBar.navAnnual
                    onClicked: toolBar.navigateTo(toolBar.navAnnual, true)
                }
            }

            ToolbarDivider {
                theme: toolBar.theme
                visible: toolBar.showDividerForGroup(toolsGroup.visible, appGroup.visible)
            }

            ToolbarGroup {
                id: appGroup
                visible: toolBar.settingsViewModel.toolbarShowSettings
                theme: toolBar.theme
                label: qsTr("Application")
                active: toolBar.appGroupActive
                iconRowHeight: toolBar.iconRowHeight

                Controls.IconButton {
                    visible: toolBar.settingsViewModel.toolbarShowSettings
                    Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                    Layout.preferredHeight: appGroup.actionHeight
                    svgSource: toolBar.assetUrl("settings.svg")
                    label: qsTr("Settings")
                    active: toolBar.navigation.activeSection === toolBar.navSettings
                    onClicked: toolBar.navigateTo(toolBar.navSettings, false)
                }
            }

            ToolbarDivider {
                theme: toolBar.theme
                visible: appGroup.visible
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }
}
