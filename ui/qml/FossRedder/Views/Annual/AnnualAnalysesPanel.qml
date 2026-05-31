/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualAnalysesPanel.qml
 * @brief Provides the Annual analyses assignment panel.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var annualState

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight
    Layout.preferredHeight: root.theme.viewSelectionPanelPreferredHeight
    contentSpacing: root.theme.spacingSmall

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true

            Rectangle {
                Layout.preferredWidth: root.theme.formLabelWidth
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
                id: addAnalysisCombo
                objectName: "annualAddAnalysisComboBox"
                Layout.fillWidth: true
                model: root.annualState.availableAnalysisRows
                textRole: "display"
                currentIndex: model.length > 0 ? 0 : -1
            }

            Controls.AddButton {
                objectName: "annualAddAnalysisButton"
                Layout.preferredWidth: root.theme.viewActionButtonWidth / 2
                Layout.minimumWidth: root.theme.viewActionButtonWidth / 2
                Layout.maximumWidth: root.theme.viewActionButtonWidth / 2
                Layout.preferredHeight: root.theme.controlHeight
                Layout.minimumHeight: root.theme.controlHeight
                Layout.maximumHeight: root.theme.controlHeight
                textColor: root.theme.textMuted
                enabled: addAnalysisCombo.currentIndex >= 0
                onClicked: root.annualState.addAvailableAnalysisAtIndex(addAnalysisCombo.currentIndex)
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight
            radius: root.theme.radius
            color: root.theme.surfaceAlt
            border.width: root.theme.borderWidthThin
            border.color: root.theme.border

            Flickable {
                id: analysisScroll
                anchors.fill: parent
                anchors.margins: root.theme.spacingSmall
                clip: true
                contentWidth: width
                contentHeight: Math.max(analysisList.implicitHeight, analysisScroll.height)

                ScrollBar.vertical: Controls.AppScrollBar {}

                Column {
                    id: analysisList
                    width: analysisScroll.width
                    spacing: root.theme.spacingSmall

                    Repeater {
                        model: root.annualState.assignedAnalysisRows

                        delegate: Rectangle {
                            id: analysisRow
                            required property var modelData
                            width: analysisList.width
                            implicitHeight: analysisContentRow.implicitHeight + (root.theme.spacingSmall * 2)
                            height: implicitHeight
                            radius: root.theme.radius
                            color: root.theme.surface
                            border.width: root.theme.borderWidthThin
                            border.color: root.theme.border

                            RowLayout {
                                id: analysisContentRow
                                anchors.fill: parent
                                anchors.margins: root.theme.spacingSmall
                                spacing: root.theme.spacingSmall

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: root.theme.formFieldWidth
                                    Layout.preferredHeight: root.theme.controlHeight
                                    radius: root.theme.radius
                                    color: root.theme.surface
                                    border.width: root.theme.borderWidthThin
                                    border.color: root.theme.borderSoft

                                    Label {
                                        anchors.fill: parent
                                        anchors.leftMargin: root.theme.spacingSmall
                                        anchors.rightMargin: root.theme.spacingSmall
                                        text: analysisRow.modelData.display
                                        color: root.theme.textPrimary
                                        horizontalAlignment: Text.AlignLeft
                                        verticalAlignment: Text.AlignVCenter
                                        elide: Text.ElideRight
                                    }
                                }

                                Rectangle {
                                    Layout.preferredWidth: root.theme.formLabelWidth
                                    Layout.preferredHeight: root.theme.controlHeight
                                    radius: root.theme.radius
                                    color: root.theme.surface
                                    border.width: root.theme.borderWidthThin
                                    border.color: root.theme.borderSoft

                                    Label {
                                        anchors.fill: parent
                                        text: analysisRow.modelData.typeLabel
                                        color: root.theme.textPrimary
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.bold: true
                                    }
                                }

                                Controls.DropdownMenu {
                                    objectName: "annualAnalysisExportFormatComboBox"
                                    Layout.preferredWidth: root.theme.formLabelWidth
                                    model: analysisRow.modelData.exportFormatOptions
                                    currentIndex: analysisRow.modelData.exportFormatIndex
                                    onActivated: root.annualState.setAnalysisExportFormat(
                                                     analysisRow.modelData.id,
                                                     model[currentIndex])
                                }

                                Controls.CompactRemoveButton {
                                    objectName: "annualRemoveAnalysisButton"
                                    onClicked: root.annualState.removeAnalysis(analysisRow.modelData.id)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
