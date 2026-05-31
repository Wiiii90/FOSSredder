/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisTableView.qml
 * @brief Provides the Analysis table preview.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    objectName: "analysisTablePreview"
    required property var theme
    required property var analysisState

    ColumnLayout {
        anchors.fill: root
        spacing: root.theme.spacingSmall

        Flickable {
            id: matrixViewport
            objectName: "analysisTableViewport"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: matrixContent.width
            contentHeight: matrixContent.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.horizontal: Controls.AppScrollBar {}
            ScrollBar.vertical: Controls.AppScrollBar {}

            ColumnLayout {
                id: matrixContent
                width: Math.max(Math.max(0, matrixViewport.width - root.theme.scrollBarGutterWidth), root.theme.analysis.table.propertyColumnWidth + Math.max(1, root.analysisState.tableContractTypes.length) * root.theme.analysis.table.amountColumnWidth + root.theme.analysis.table.totalColumnWidth)
                spacing: 0

                Rectangle {
                    visible: root.analysisState.tablePropertyRows.length > 0
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.analysis.table.rowHeight
                    color: root.theme.surfaceAlt
                    border.width: root.theme.borderWidthThin
                    border.color: root.theme.borderStrong

                    RowLayout {
                        anchors.fill: parent
                        spacing: 0

                        Label {
                            color: root.theme.textPrimary
                            text: qsTr("Property")
                            Layout.preferredWidth: root.theme.analysis.table.propertyColumnWidth
                            leftPadding: root.theme.spacingSmall
                            elide: Text.ElideRight
                        }

                        Repeater {
                            model: root.analysisState.tableContractTypes

                            Label {
                                color: root.theme.textPrimary
                                required property var modelData
                                text: modelData
                                Layout.preferredWidth: root.theme.analysis.table.amountColumnWidth
                                horizontalAlignment: Text.AlignRight
                                rightPadding: root.theme.spacingSmall
                                elide: Text.ElideRight
                            }
                        }

                        Label {
                            color: root.theme.textPrimary
                            text: qsTr("Total")
                            Layout.preferredWidth: root.theme.analysis.table.totalColumnWidth
                            Layout.minimumWidth: root.theme.analysis.table.totalColumnWidth
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                            rightPadding: root.theme.spacingSmall
                        }
                    }
                }

                Repeater {
                    visible: root.analysisState.tablePropertyRows.length > 0
                    model: root.analysisState.tablePropertyRows

                    Rectangle {
                        id: rowContainer
                        required property int index
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.preferredHeight: root.theme.analysis.table.rowHeight
                        color: index % 2 === 0 ? root.theme.surface : root.theme.surfaceAlt
                        border.width: root.theme.borderWidthThin
                        border.color: root.theme.borderLight

                        RowLayout {
                            anchors.fill: parent
                            spacing: 0

                            Label {
                                color: root.theme.textPrimary
                                text: rowContainer.modelData.propertyName
                                Layout.preferredWidth: root.theme.analysis.table.propertyColumnWidth
                                leftPadding: root.theme.spacingSmall
                                elide: Text.ElideRight
                            }

                            Repeater {
                                model: rowContainer.modelData.amounts

                                Rectangle {
                                    id: amountCell
                                    required property var modelData
                                    Layout.preferredWidth: root.theme.analysis.table.amountColumnWidth
                                    Layout.preferredHeight: root.theme.analysis.table.rowHeight
                                    color: rowContainer.index % 2 === 0 ? root.theme.surface : root.theme.surfaceAlt
                                    border.width: root.theme.borderWidthThin
                                    border.color: root.theme.borderLight

                                    Label {
                                        color: root.theme.textPrimary
                                        anchors.fill: parent
                                        text: Number(amountCell.modelData).toFixed(2)
                                        horizontalAlignment: Text.AlignRight
                                        verticalAlignment: Text.AlignVCenter
                                        rightPadding: root.theme.spacingSmall
                                    }
                                }
                            }

                            Rectangle {
                                Layout.preferredWidth: root.theme.analysis.table.totalColumnWidth
                                Layout.minimumWidth: root.theme.analysis.table.totalColumnWidth
                                Layout.fillWidth: true
                                Layout.preferredHeight: root.theme.analysis.table.rowHeight
                                color: root.theme.surfaceAlt
                                border.width: root.theme.borderWidthThin
                                border.color: root.theme.borderStrong

                                Label {
                                    color: root.theme.textPrimary
                                    anchors.fill: parent
                                    text: Number(rowContainer.modelData.total).toFixed(2)
                                    horizontalAlignment: Text.AlignRight
                                    verticalAlignment: Text.AlignVCenter
                                    rightPadding: root.theme.spacingSmall
                                    font.bold: true
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    visible: root.analysisState.tablePropertyRows.length > 0
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.analysis.table.rowHeight
                    color: root.theme.surfaceAlt
                    border.width: root.theme.borderWidthThin
                    border.color: root.theme.borderStrong

                    RowLayout {
                        anchors.fill: parent
                        spacing: 0

                        Label {
                            text: qsTr("Total")
                            Layout.preferredWidth: root.theme.analysis.table.propertyColumnWidth
                            leftPadding: root.theme.spacingSmall
                            verticalAlignment: Text.AlignVCenter
                            font.bold: true
                            color: root.theme.textPrimary
                        }

                        Repeater {
                            model: root.analysisState.tableContractTypes.length

                            Rectangle {
                                Layout.preferredWidth: root.theme.analysis.table.amountColumnWidth
                                Layout.preferredHeight: root.theme.analysis.table.rowHeight
                                color: root.theme.surfaceAlt
                                border.width: root.theme.borderWidthThin
                                border.color: root.theme.borderStrong

                                Label {
                                    color: root.theme.textPrimary
                                    anchors.fill: parent
                                    text: ""
                                }
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: root.theme.analysis.table.totalColumnWidth
                            Layout.minimumWidth: root.theme.analysis.table.totalColumnWidth
                            Layout.fillWidth: true
                            Layout.preferredHeight: root.theme.analysis.table.rowHeight
                            color: root.theme.surfaceAlt
                            border.width: root.theme.borderWidthThin
                            border.color: root.theme.borderStrong

                            Label {
                                color: root.theme.textPrimary
                                anchors.fill: parent
                                text: root.analysisState.tableGrandTotal.toFixed(2)
                                rightPadding: root.theme.spacingSmall
                                horizontalAlignment: Text.AlignRight
                                verticalAlignment: Text.AlignVCenter
                                font.bold: true
                            }
                        }
                    }
                }

                Rectangle {
                    visible: root.analysisState.tablePropertyRows.length === 0
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.analysis.table.rowHeight * 2
                    color: root.theme.surfaceAlt
                    border.width: root.theme.borderWidthThin
                    border.color: root.theme.borderLight

                    Label {
                        anchors.centerIn: parent
                        text: qsTr("No table preview available")
                        color: root.theme.textMuted
                    }
                }
            }
        }
    }
}
