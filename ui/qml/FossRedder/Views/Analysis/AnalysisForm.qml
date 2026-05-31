/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisForm.qml
 * @brief Provides the Analysis form surface.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Analysis 1.0 as Analysis

Item {
    id: root
    required property var theme
    required property var analysisState

    readonly property bool isEdit: root.analysisState.isEdit
    readonly property int filterWorkspaceIndex: root.analysisState.filterWorkspaceIndex
    readonly property string allocatableMode: root.analysisState.allocatableMode
    readonly property var selectedPropertyIds: root.analysisState.selectedPropertyIds
    readonly property var selectedContractTypes: root.analysisState.selectedContractTypes
    readonly property string pendingAdjustmentsJson: root.analysisState.pendingAdjustmentsJson

    ColumnLayout {
        anchors.fill: root
        anchors.margins: root.theme.pageContentMargin
        anchors.bottomMargin: 0
        spacing: root.theme.spacingSmall

        Flickable {
            id: analysisScroll
            Layout.fillWidth: true
            Layout.fillHeight: root.analysisState.isEdit
            Layout.minimumHeight: 0
            Layout.preferredHeight: root.analysisState.isEdit ? -1 : analysisContent.implicitHeight
            clip: true
            contentWidth: width
            contentHeight: root.analysisState.isEdit ? height : analysisContent.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Controls.AppScrollBar { hidden: true }

            ColumnLayout {
                id: analysisContent
                width: analysisScroll.width
                height: root.analysisState.isEdit ? analysisScroll.height : implicitHeight
                spacing: root.theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        color: root.theme.textPrimary
                        text: qsTr("Analysis Name")
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }

                    Controls.TextField {
                        id: nameField
                        objectName: "analysisNameField"
                        Layout.fillWidth: true
                        text: root.analysisState.name
                        onTextChanged: root.analysisState.name = text
                    }
                }

                Controls.Panel {
                    visible: !root.analysisState.isEdit
                    Layout.fillWidth: true
                    Layout.preferredHeight: analysisTypeColumn.implicitHeight + (root.theme.panelPadding * 2)

                    ColumnLayout {
                        id: analysisTypeColumn
                        Layout.fillWidth: true
                        spacing: root.theme.spacingSmall

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                color: root.theme.textPrimary
                                text: qsTr("Analysis Type")
                                Layout.preferredWidth: root.theme.formLabelWidth
                            }

                            Controls.DropdownMenu {
                                id: mainTypeCombo
                                objectName: "analysisMainTypeComboBox"
                                Layout.fillWidth: true
                                model: [qsTr("Plot"), qsTr("Table")]
                                currentIndex: root.analysisState.mainTypeIndex
                                onActivated: root.analysisState.mainTypeIndex = currentIndex
                            }
                        }

                        RowLayout {
                            visible: root.analysisState.mainTypeIndex === 0
                            Layout.fillWidth: true

                            Label {
                                color: root.theme.textPrimary
                                text: qsTr("Plot Subtype")
                                Layout.preferredWidth: root.theme.formLabelWidth
                            }

                            Controls.DropdownMenu {
                                id: plotSubtypeCombo
                                objectName: "analysisPlotSubtypeComboBox"
                                Layout.fillWidth: true
                                Layout.maximumWidth: root.width - root.theme.formLabelWidth - root.theme.panelContentSafeWidthOffset
                                model: root.analysisState.plotTypeOptions
                                textRole: "label"
                                currentIndex: root.analysisState.plotSubtypeIndex
                                onActivated: root.analysisState.plotSubtypeIndex = currentIndex
                            }
                        }
                    }
                }

                Controls.Panel {
                    visible: root.analysisState.isEdit
                    Layout.fillWidth: true
                    Layout.preferredHeight: exportOptionsColumn.implicitHeight + (root.theme.panelPadding * 2)

                    ColumnLayout {
                        id: exportOptionsColumn
                        Layout.fillWidth: true
                        spacing: root.theme.spacingSmall

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                color: root.theme.textPrimary
                                text: qsTr("Export Format")
                                Layout.preferredWidth: root.theme.formLabelWidth
                            }

                            Controls.DropdownMenu {
                                id: exportFormatCombo
                                objectName: "analysisExportFormatComboBox"
                                Layout.preferredWidth: root.theme.formFieldWidth
                                model: root.analysisState.exportFormatOptions
                                textRole: "label"
                                currentIndex: root.analysisState.exportFormatIndex
                                onActivated: root.analysisState.exportFormatIndex = currentIndex
                            }

                            Item {
                                Layout.fillWidth: true
                            }
                        }

                        Item {
                            id: includeCalcAdjustmentsRow
                            objectName: "analysisIncludeCalcAdjustmentsRow"
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.max(includeCalcAdjustmentsCheckBox.implicitHeight, includeCalcAdjustmentsLabel.implicitHeight)

                            RowLayout {
                                anchors.fill: parent

                                Controls.CheckBox {
                                    id: includeCalcAdjustmentsCheckBox
                                    objectName: "analysisIncludeCalcAdjustmentsCheckBox"
                                    Layout.fillWidth: false
                                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                    checked: root.analysisState.includeCalcAdjustments
                                }

                                Label {
                                    id: includeCalcAdjustmentsLabel
                                    color: root.theme.textPrimary
                                    text: qsTr("Include Calc Adjustments")
                                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                }

                                Item {
                                    Layout.fillWidth: true
                                }
                            }

                            MouseArea {
                                objectName: "analysisIncludeCalcAdjustmentsMouseArea"
                                anchors.fill: parent
                                z: 1
                                onClicked: root.analysisState.includeCalcAdjustments = !root.analysisState.includeCalcAdjustments
                            }
                        }
                    }
                }

                Controls.Panel {
                    visible: root.analysisState.isEdit
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 0
                    Layout.preferredHeight: -1

                    Loader {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        sourceComponent: root.analysisState.currentResultIsTable ? tableComp : plotComp
                    }
                }
            }
        }

        Controls.Panel {
            visible: !root.analysisState.isEdit && root.analysisState.filterEditMode
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight + root.theme.controlHeight + root.theme.spacingSmall * 3

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: root.analysisState.filterWorkspaceIndex

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Flickable {
                        id: filterSpecScroll
                        anchors.fill: parent
                        clip: true
                        contentWidth: width
                        contentHeight: filterSpecContent.implicitHeight
                        boundsBehavior: Flickable.StopAtBounds

                        ScrollBar.vertical: Controls.AppScrollBar {}

                        ColumnLayout {
                            id: filterSpecContent
                            width: Math.max(0, filterSpecScroll.width - root.theme.scrollBarGutterWidth)
                            spacing: root.theme.spacingSmall

                            Item {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 1
                            }

                            Analysis.AnalysisDateFilter {
                                id: dateFilter
                                theme: root.theme
                                analysisState: root.analysisState
                                Layout.fillWidth: true
                                Layout.preferredHeight: dateFilter.implicitHeight
                            }

                            Analysis.AnalysisPropertyFilter {
                                theme: root.theme
                                analysisState: root.analysisState
                                Layout.fillWidth: true
                            }

                            Analysis.AnalysisContractTypeFilter {
                                theme: root.theme
                                analysisState: root.analysisState
                                Layout.fillWidth: true
                            }

                            Analysis.AnalysisAllocatableFilter {
                                id: allocatableFilterPanel
                                theme: root.theme
                                analysisState: root.analysisState
                                mode: root.analysisState.allocatableMode
                                Layout.fillWidth: true
                                Layout.preferredHeight: allocatableFilterPanel.implicitHeight
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.preferredHeight: root.theme.spacingSmall
                            }
                        }
                    }
                }

                Analysis.AnalysisTransactionsPanel {
                    theme: root.theme
                    analysisState: root.analysisState
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }

    Component {
        id: plotComp

        Analysis.AnalysisPlotView {
            theme: root.theme
            analysisState: root.analysisState
        }
    }

    Component {
        id: tableComp

        Analysis.AnalysisTableView {
            theme: root.theme
            analysisState: root.analysisState
        }
    }
}
