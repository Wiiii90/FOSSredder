/**
 * @file ui/qml/FossRedder/Components/Toolbar.qml
 * @brief Provides the Toolbar component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: toolBar
    required property var shellNavigationState
    required property var settingsViewModel
    required property var theme
    readonly property int navActors: toolBar.shellNavigationState.actorSection
    readonly property int navProperties: toolBar.shellNavigationState.propertySection
    readonly property int navContracts: toolBar.shellNavigationState.contractSection
    readonly property int navBooking: toolBar.shellNavigationState.bookingSection
    readonly property int navImport: toolBar.shellNavigationState.importSection
    readonly property int navExport: toolBar.shellNavigationState.exportSection
    readonly property int navSettings: toolBar.shellNavigationState.settingsSection
    readonly property int navAnalysis: toolBar.shellNavigationState.analysisSection
    readonly property int navAnnual: toolBar.shellNavigationState.annualSection
    implicitHeight: toolBar.theme.toolbarHeight
    property int iconRowHeight: Math.round(implicitHeight * 0.55)

    function assetUrl(fileName) {
        return Qt.resolvedUrl("../assets/" + fileName)
    }

    function navigateTo(section, clearSelection) {
        toolBar.shellNavigationState.navigateToSection(section, clearSelection)
    }

    function navigateToImportHome() {
        toolBar.shellNavigationState.navigateToImportHome()
    }

    function navigateToBookingCreate() {
        toolBar.shellNavigationState.navigateToBookingCreate()
    }

    function showDividerForGroup(visibleGroup, hasVisibleAfter) {
        return visibleGroup && hasVisibleAfter
    }

    Rectangle {
        id: bg
        anchors.left: parent.left
        anchors.right: parent.right
        height: toolBar.implicitHeight
        color: toolBar.theme.toolbarBackground
        border.width: toolBar.theme.borderWidthThin
        border.color: toolBar.theme.toolbarBorder
            clip: false

        RowLayout {
            id: rootRow
            anchors.fill: parent
            spacing: toolBar.theme.spacing + toolBar.theme.margins
            Layout.alignment: Qt.AlignVCenter

            ColumnLayout {
                id: fileGroup
                visible: toolBar.settingsViewModel.toolbarShowImport
                         || toolBar.settingsViewModel.toolbarShowExport
                spacing: toolBar.theme.toolbarSectionSpacing
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: fileIcons
                    spacing: toolBar.theme.toolbarGroupSpacing
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.preferredHeight: toolBar.iconRowHeight

                    Controls.IconButton {
                        visible: toolBar.settingsViewModel.toolbarShowImport
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: fileIcons.height
                        svgSource: toolBar.assetUrl("import.svg")
                        label: qsTr("Import")
                        active: toolBar.shellNavigationState.activeSection === toolBar.navImport
                        onClicked: toolBar.navigateToImportHome()
                    }
                    Controls.IconButton {
                        visible: toolBar.settingsViewModel.toolbarShowExport
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: fileIcons.height
                        svgSource: toolBar.assetUrl("export.svg")
                        label: qsTr("Export")
                        active: toolBar.shellNavigationState.activeSection === toolBar.navExport
                        onClicked: toolBar.navigateTo(toolBar.navExport, false)
                    }
                }

                Text {
                    id: groupLabelFile
                    text: qsTr("File")
                    color: toolBar.theme.textMuted
                    font.pointSize: toolBar.theme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle {
                Layout.preferredWidth: toolBar.theme.borderWidthThin
                Layout.fillHeight: true
                color: toolBar.theme.divider
                Layout.alignment: Qt.AlignVCenter
                visible: toolBar.showDividerForGroup(fileGroup.visible,
                    domainGroup.visible || toolsGroup.visible || appGroup.visible)
            }

            ColumnLayout {
                id: domainGroup
                visible: toolBar.settingsViewModel.toolbarShowBooking
                         || toolBar.settingsViewModel.toolbarShowActors
                         || toolBar.settingsViewModel.toolbarShowProperties
                         || toolBar.settingsViewModel.toolbarShowContracts
                spacing: toolBar.theme.toolbarSectionSpacing
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: domainIcons
                    spacing: toolBar.theme.toolbarGroupSpacing
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.preferredHeight: toolBar.iconRowHeight

                    Controls.IconButton {
                        visible: toolBar.settingsViewModel.toolbarShowBooking
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: domainIcons.height
                        svgSource: toolBar.assetUrl("booking.svg")
                        label: qsTr("Booking")
                        active: toolBar.shellNavigationState.activeSection === toolBar.navBooking
                        onClicked: toolBar.navigateToBookingCreate()
                    }
                    Controls.IconButton {
                        visible: toolBar.settingsViewModel.toolbarShowActors
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: domainIcons.height
                        svgSource: toolBar.assetUrl("actor.svg")
                        label: qsTr("Actor")
                        active: toolBar.shellNavigationState.activeSection === toolBar.navActors
                        onClicked: toolBar.navigateTo(toolBar.navActors, true)
                    }
                    Controls.IconButton {
                        visible: toolBar.settingsViewModel.toolbarShowProperties
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: domainIcons.height
                        svgSource: toolBar.assetUrl("property.svg")
                        label: qsTr("Property")
                        active: toolBar.shellNavigationState.activeSection === toolBar.navProperties
                        onClicked: toolBar.navigateTo(toolBar.navProperties, true)
                    }
                    Controls.IconButton {
                        visible: toolBar.settingsViewModel.toolbarShowContracts
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: domainIcons.height
                        svgSource: toolBar.assetUrl("contract.svg")
                        label: qsTr("Contract")
                        active: toolBar.shellNavigationState.activeSection === toolBar.navContracts
                        onClicked: toolBar.navigateTo(toolBar.navContracts, true)
                    }
                }

                Text {
                    id: groupLabelDomain
                    text: qsTr("Domain")
                    color: toolBar.theme.textMuted
                    font.pointSize: toolBar.theme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle {
                Layout.preferredWidth: toolBar.theme.borderWidthThin
                Layout.fillHeight: true
                color: toolBar.theme.divider
                Layout.alignment: Qt.AlignVCenter
                visible: toolBar.showDividerForGroup(domainGroup.visible,
                    toolsGroup.visible || appGroup.visible)
            }

            ColumnLayout {
                id: toolsGroup
                visible: toolBar.settingsViewModel.toolbarShowAnalysis
                         || toolBar.settingsViewModel.toolbarShowAnnual
                spacing: toolBar.theme.toolbarSectionSpacing
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: toolsIcons
                    spacing: toolBar.theme.toolbarGroupSpacing
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.preferredHeight: toolBar.iconRowHeight

                    Controls.IconButton {
                        visible: toolBar.settingsViewModel.toolbarShowAnalysis
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: toolsIcons.height
                        svgSource: toolBar.assetUrl("analysis.svg")
                        label: qsTr("Analysis")
                        active: toolBar.shellNavigationState.activeSection === toolBar.navAnalysis
                        onClicked: toolBar.navigateTo(toolBar.navAnalysis, true)
                    }
                    Controls.IconButton {
                        visible: toolBar.settingsViewModel.toolbarShowAnnual
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: toolsIcons.height
                        svgSource: toolBar.assetUrl("annual.svg")
                        label: qsTr("Annual")
                        active: toolBar.shellNavigationState.activeSection === toolBar.navAnnual
                        onClicked: toolBar.navigateTo(toolBar.navAnnual, true)
                    }
                }

                Text {
                    id: groupLabelTools
                    text: qsTr("Tools")
                    color: toolBar.theme.textMuted
                    font.pointSize: toolBar.theme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle {
                Layout.preferredWidth: toolBar.theme.borderWidthThin
                Layout.fillHeight: true
                color: toolBar.theme.divider
                Layout.alignment: Qt.AlignVCenter
                visible: toolBar.showDividerForGroup(toolsGroup.visible, appGroup.visible)
            }

            ColumnLayout {
                id: appGroup
                visible: toolBar.settingsViewModel.toolbarShowSettings
                spacing: toolBar.theme.toolbarSectionSpacing
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: appIcons
                    spacing: toolBar.theme.toolbarGroupSpacing
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.preferredHeight: toolBar.iconRowHeight

                    Controls.IconButton {
                        visible: toolBar.settingsViewModel.toolbarShowSettings
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: appIcons.height
                        svgSource: toolBar.assetUrl("settings.svg")
                        label: qsTr("Settings")
                        active: toolBar.shellNavigationState.activeSection === toolBar.navSettings
                        onClicked: toolBar.navigateTo(toolBar.navSettings, false)
    }
                }

                Text {
                    id: groupLabelApp
                    text: qsTr("Application")
                    color: toolBar.theme.textMuted
                    font.pointSize: toolBar.theme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle {
                Layout.preferredWidth: toolBar.theme.borderWidthThin
                Layout.fillHeight: true
                color: toolBar.theme.divider
                Layout.alignment: Qt.AlignVCenter
                visible: appGroup.visible
            }

            Item { Layout.fillWidth: true }
        }
    }
}
