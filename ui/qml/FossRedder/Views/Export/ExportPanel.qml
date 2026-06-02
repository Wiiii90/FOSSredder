/**
 * @file ui/qml/FossRedder/Views/Export/ExportPanel.qml
 * @brief Provides the ExportPanel component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var exportViewModel
    required property var theme

    readonly property int exportTypeColumnWidth: root.theme.exportView.panel.exportTypeColumnWidth
    readonly property int removeColumnWidth: root.theme.viewCompactActionButtonSize
    readonly property int leadingColumnWidth: root.theme.viewCompactActionButtonSize
    readonly property int kindColumnWidth: root.theme.exportView.panel.kindColumnWidth
    readonly property int analysisNameMinWidth: root.theme.exportView.panel.analysisNameMinWidth

    contentSpacing: root.theme.spacingSmall

    RowLayout {
        Layout.fillWidth: true

        Controls.Button {
            objectName: "exportAddAnnualModeButton"
            text: qsTr("Annual")
            bordered: true
            filled: root.exportViewModel.addMode === "annual"
            fillColor: filled ? root.theme.subtlePrimaryFill : root.theme.surface
            textColor: root.theme.textPrimary
            Layout.preferredWidth: root.theme.exportView.panel.addModeButtonWidth
            Layout.preferredHeight: root.theme.controlHeight
            onClicked: root.exportViewModel.addMode = "annual"
        }

        Controls.Button {
            objectName: "exportAddAnalysisModeButton"
            text: qsTr("Analysis")
            bordered: true
            filled: root.exportViewModel.addMode === "analysis"
            fillColor: filled ? root.theme.subtlePrimaryFill : root.theme.surface
            textColor: root.theme.textPrimary
            Layout.preferredWidth: root.theme.exportView.panel.addModeButtonWidth
            Layout.preferredHeight: root.theme.controlHeight
            onClicked: root.exportViewModel.addMode = "analysis"
        }

        Controls.DropdownMenu {
            objectName: "exportAddObjectComboBox"
            Layout.fillWidth: true
            Layout.preferredWidth: root.theme.formFieldWidth
            model: root.exportViewModel.addRows
            textRole: root.exportViewModel.addTextRole
            currentIndex: root.exportViewModel.pendingIndex
            onActivated: function(index) { root.exportViewModel.selectPendingRow(index) }
        }

        Controls.AddButton {
            objectName: "exportAddEntryButton"
            Layout.preferredWidth: root.theme.exportView.panel.addButtonWidth
            Layout.minimumWidth: root.theme.exportView.panel.addButtonWidth
            Layout.maximumWidth: root.theme.exportView.panel.addButtonWidth
            Layout.preferredHeight: root.theme.controlHeight
            Layout.minimumHeight: root.theme.controlHeight
            Layout.maximumHeight: root.theme.controlHeight
            enabled: root.exportViewModel.canAddEntry
            onClicked: root.exportViewModel.addPendingEntry()
        }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: root.theme.exportView.panel.objectListMinHeight
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: root.theme.borderWidthThin
        border.color: root.theme.border

        Flickable {
            id: objectsFlick
            anchors.fill: parent
            anchors.margins: root.theme.spacing
            clip: true
            contentWidth: width
            contentHeight: Math.max(contentColumn.implicitHeight, objectsFlick.height)

            ScrollBar.vertical: Controls.AppScrollBar {}

            Column {
                id: contentColumn
                width: objectsFlick.width
                spacing: root.theme.spacing

                Item {
                    width: contentColumn.width
                    height: root.exportViewModel.exportEntries.length === 0
                            ? Math.max(objectsFlick.height, dropContent.implicitHeight)
                            : 0
                    visible: root.exportViewModel.exportEntries.length === 0

                    ColumnLayout {
                        id: dropContent
                        anchors.fill: parent
                        anchors.margins: root.theme.spacing
                        spacing: root.theme.spacingSmall

                        Image {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: root.theme.viewSectionIconSize
                            Layout.preferredHeight: root.theme.viewSectionIconSize
                            source: Qt.resolvedUrl("../../assets/export.svg")
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            opacity: 0.85
                        }

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("Add Annuals here")
                            color: root.theme.textPrimary
                            horizontalAlignment: Text.AlignHCenter
                            font.bold: true
                            wrapMode: Text.WordWrap
                        }

                        Item { Layout.fillHeight: true }
                    }
                }

                Repeater {
                    model: root.exportViewModel.exportEntries

                    delegate: Loader {
                        required property var modelData
                        required property int index
                        width: contentColumn.width
                        sourceComponent: modelData.isAnnual ? annualEntryComponent : standaloneAnalysisComponent
                        onLoaded: {
                            if (!item) return
                            item.entryData = modelData
                            item.entryIndex = index
                            item.width = width
                        }
                    }
                }
            }
        }
    }

    Component {
        id: annualEntryComponent

        Item {
            id: annualEntryItem
            property var entryData: null
            property int entryIndex: -1
            property var annualData: entryData
            width: contentColumn.width
            implicitHeight: annualPanelColumn.implicitHeight

            Column {
                id: annualPanelColumn
                width: parent.width
                spacing: root.theme.spacing

                Rectangle {
                    width: parent.width
                    implicitHeight: annualCardColumn.implicitHeight + (root.theme.spacing * 2)
                    height: implicitHeight
                    radius: root.theme.radius
                    color: "transparent"
                    border.width: root.theme.borderWidthThin
                    border.color: root.theme.borderSoft

                    Column {
                        id: annualCardColumn
                        anchors.fill: parent
                        anchors.margins: root.theme.spacing
                        spacing: root.theme.spacing

                        RowLayout {
                            width: parent.width
                            spacing: root.theme.spacing

                            Controls.DisclosureButton {
                                objectName: "exportAnnualCollapseButton"
                                expanded: !annualEntryItem.annualData.collapsed
                                Layout.preferredWidth: root.leadingColumnWidth
                                Layout.minimumWidth: root.leadingColumnWidth
                                Layout.maximumWidth: root.leadingColumnWidth
                                Layout.preferredHeight: root.theme.viewCompactActionButtonSize
                                Layout.minimumHeight: root.theme.viewCompactActionButtonSize
                                Layout.maximumHeight: root.theme.viewCompactActionButtonSize
                                onClicked: root.exportViewModel.updateAnnualCollapsed(
                                               annualEntryItem.entryIndex,
                                               !annualEntryItem.annualData.collapsed)
                            }

                            Rectangle {
                                Layout.preferredWidth: root.kindColumnWidth
                                Layout.preferredHeight: root.theme.controlHeight
                                radius: root.theme.radius
                                color: root.theme.surface
                                border.width: root.theme.borderWidthThin
                                border.color: root.theme.borderSoft

                                Label {
                                    anchors.fill: parent
                                    text: qsTr("Annual")
                                    color: root.theme.textPrimary
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    font.bold: true
                                }
                            }

                            Controls.DropdownMenu {
                                objectName: "exportAnnualObjectComboBox"
                                Layout.fillWidth: true
                                model: root.exportViewModel.annualRows
                                textRole: "display"
                                currentIndex: annualEntryItem.annualData.annualIndex
                                onActivated: root.exportViewModel.updateAnnualEntryAtIndex(
                                                 annualEntryItem.entryIndex,
                                                 currentIndex)
                            }

                            Item {
                                Layout.preferredWidth: root.exportTypeColumnWidth
                                Layout.minimumWidth: root.exportTypeColumnWidth
                                Layout.maximumWidth: root.exportTypeColumnWidth
                            }

                            Controls.CompactRemoveButton {
                                objectName: "exportRemoveAnnualButton"
                                Layout.preferredWidth: root.removeColumnWidth
                                Layout.minimumWidth: root.removeColumnWidth
                                Layout.maximumWidth: root.removeColumnWidth
                                onClicked: root.exportViewModel.removeEntry(annualEntryItem.entryIndex)
                            }
                        }

                        Column {
                            id: annualAnalysesColumn
                            width: parent.width
                            spacing: root.theme.spacing
                            visible: !annualEntryItem.annualData.collapsed

                            Label {
                                width: parent.width
                                visible: !annualEntryItem.annualData.analyses
                                         || annualEntryItem.annualData.analyses.length === 0
                                text: qsTr("No analyses assigned")
                                color: root.theme.textMuted
                                wrapMode: Text.WordWrap
                                leftPadding: root.leadingColumnWidth
                                             + root.kindColumnWidth
                                             + root.theme.spacing * 2
                            }

                            Repeater {
                                model: annualEntryItem.annualData.analyses

                                delegate: Rectangle {
                                    id: annualAnalysisEntry
                                    required property var modelData
                                    required property int index
                                    width: annualAnalysesColumn.width
                                    implicitHeight: annualAnalysisRow.implicitHeight
                                                    + (root.theme.spacing * 2)
                                    height: implicitHeight
                                    radius: root.theme.radius
                                    color: "transparent"
                                    border.color: root.theme.borderSoft
                                    border.width: root.theme.borderWidthThin

                                    RowLayout {
                                        id: annualAnalysisRow
                                        anchors.fill: parent
                                        anchors.margins: root.theme.spacing
                                        spacing: root.theme.spacing

                                        Item {
                                            Layout.preferredWidth: root.leadingColumnWidth
                                            Layout.minimumWidth: root.leadingColumnWidth
                                            Layout.maximumWidth: root.leadingColumnWidth
                                        }

                                        Rectangle {
                                            Layout.preferredWidth: root.kindColumnWidth
                                            Layout.preferredHeight: root.theme.controlHeight
                                            radius: root.theme.radius
                                            color: root.theme.surface
                                            border.width: root.theme.borderWidthThin
                                            border.color: root.theme.borderSoft

                                            Label {
                                                anchors.fill: parent
                                                text: qsTr("Analysis")
                                                color: root.theme.textPrimary
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                                font.bold: true
                                            }
                                        }

                                        Rectangle {
                                            Layout.fillWidth: true
                                            Layout.minimumWidth: root.analysisNameMinWidth
                                            Layout.preferredHeight: root.theme.controlHeight
                                            radius: root.theme.radius
                                            color: root.theme.surface
                                            border.width: root.theme.borderWidthThin
                                            border.color: root.theme.borderSoft

                                            Label {
                                                anchors.fill: parent
                                                anchors.leftMargin: root.theme.spacing
                                                anchors.rightMargin: root.theme.spacing
                                                text: annualAnalysisEntry.modelData.objectName
                                                      && annualAnalysisEntry.modelData.objectName.length > 0
                                                      ? annualAnalysisEntry.modelData.objectName
                                                      : qsTr("(unassigned)")
                                                color: root.theme.textPrimary
                                                horizontalAlignment: Text.AlignLeft
                                                verticalAlignment: Text.AlignVCenter
                                                elide: Text.ElideRight
                                            }
                                        }

                                        Controls.DropdownMenu {
                                            id: annualExportTypeDropdown
                                            objectName: "exportAnnualAnalysisExportTypeComboBox"
                                            Layout.preferredWidth: root.exportTypeColumnWidth
                                            Layout.minimumWidth: root.exportTypeColumnWidth
                                            Layout.maximumWidth: root.exportTypeColumnWidth
                                            model: annualAnalysisEntry.modelData.exportTypeOptions
                                            currentIndex: Math.max(0, annualAnalysisEntry.modelData.exportTypeIndex)
                                            onActivated: root.exportViewModel.updateAnnualAnalysisExportType(
                                                             annualEntryItem.entryIndex,
                                                             annualAnalysisEntry.index,
                                                             annualExportTypeDropdown.currentText)
                                        }

                                        Item {
                                            Layout.preferredWidth: root.removeColumnWidth
                                            Layout.minimumWidth: root.removeColumnWidth
                                            Layout.maximumWidth: root.removeColumnWidth
                                            Layout.preferredHeight: root.theme.viewCompactActionButtonSize
                                            Layout.minimumHeight: root.theme.viewCompactActionButtonSize
                                            Layout.maximumHeight: root.theme.viewCompactActionButtonSize
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: standaloneAnalysisComponent

        Rectangle {
            id: standaloneAnalysisEntry
            property var entryData: null
            property int entryIndex: -1
            property var modelData: entryData
            width: contentColumn.width
            implicitHeight: standaloneAnalysisRow.implicitHeight + (root.theme.spacing * 2)
            height: implicitHeight
            radius: root.theme.radius
            color: "transparent"
            border.color: root.theme.borderSoft
            border.width: root.theme.borderWidthThin

            RowLayout {
                id: standaloneAnalysisRow
                anchors.fill: parent
                anchors.margins: root.theme.spacing
                spacing: root.theme.spacing

                Item {
                    Layout.preferredWidth: root.leadingColumnWidth
                    Layout.minimumWidth: root.leadingColumnWidth
                    Layout.maximumWidth: root.leadingColumnWidth
                    Layout.preferredHeight: root.theme.viewCompactActionButtonSize
                    Layout.minimumHeight: root.theme.viewCompactActionButtonSize
                    Layout.maximumHeight: root.theme.viewCompactActionButtonSize
                }

                Rectangle {
                    Layout.preferredWidth: root.kindColumnWidth
                    Layout.preferredHeight: root.theme.controlHeight
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.width: root.theme.borderWidthThin
                    border.color: root.theme.borderSoft

                    Label {
                        anchors.fill: parent
                        text: qsTr("Analysis")
                        color: root.theme.textPrimary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.bold: true
                    }
                }

                Controls.DropdownMenu {
                    objectName: "exportAnalysisObjectComboBox"
                    Layout.fillWidth: true
                    Layout.minimumWidth: root.analysisNameMinWidth
                    model: root.exportViewModel.analysisRows
                    textRole: "name"
                    currentIndex: standaloneAnalysisEntry.modelData.analysisIndex
                    onActivated: root.exportViewModel.updateStandaloneAnalysisAtIndex(
                                     standaloneAnalysisEntry.entryIndex,
                                     currentIndex)
                }

                Controls.DropdownMenu {
                    id: standaloneExportTypeDropdown
                    objectName: "exportStandaloneAnalysisExportTypeComboBox"
                    Layout.preferredWidth: root.exportTypeColumnWidth
                    Layout.minimumWidth: root.exportTypeColumnWidth
                    Layout.maximumWidth: root.exportTypeColumnWidth
                    model: standaloneAnalysisEntry.modelData.exportTypeOptions
                    currentIndex: Math.max(0, standaloneAnalysisEntry.modelData.exportTypeIndex)
                    onActivated: root.exportViewModel.updateStandaloneAnalysisExportType(
                                     standaloneAnalysisEntry.entryIndex,
                                     standaloneExportTypeDropdown.currentText)
                }

                Controls.CompactRemoveButton {
                    objectName: "exportRemoveAnalysisButton"
                    Layout.preferredWidth: root.removeColumnWidth
                    Layout.minimumWidth: root.removeColumnWidth
                    Layout.maximumWidth: root.removeColumnWidth
                    onClicked: root.exportViewModel.removeEntry(standaloneAnalysisEntry.entryIndex)
                }
            }
        }
    }
}
