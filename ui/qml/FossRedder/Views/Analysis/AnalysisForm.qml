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
    required property var analysisViewModel

    readonly property bool isEdit: root.analysisViewModel.isEdit
    readonly property int filterContentIndex: root.analysisViewModel.filterContentIndex
    readonly property string allocatableMode: root.analysisViewModel.allocatableMode
    readonly property var selectedPropertyIds: root.analysisViewModel.selectedPropertyIds
    readonly property var selectedContractTypes: root.analysisViewModel.selectedContractTypes

    ColumnLayout {
        anchors.fill: root
        anchors.margins: root.theme.pageContentMargin
        anchors.bottomMargin: 0
        spacing: root.theme.spacingSmall

        Flickable {
            id: analysisScroll
            Layout.fillWidth: true
            Layout.fillHeight: root.analysisViewModel.isEdit
            Layout.minimumHeight: 0
            Layout.preferredHeight: root.analysisViewModel.isEdit ? -1 : analysisContent.implicitHeight
            clip: true
            contentWidth: width
            contentHeight: root.analysisViewModel.isEdit ? height : analysisContent.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Controls.AppScrollBar { hidden: true }

            ColumnLayout {
                id: analysisContent
                width: analysisScroll.width
                height: root.analysisViewModel.isEdit ? analysisScroll.height : implicitHeight
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
                        text: root.analysisViewModel.name
                        onTextChanged: root.analysisViewModel.name = text
                    }
                }

                Controls.Panel {
                    visible: !root.analysisViewModel.isEdit
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
                                currentIndex: root.analysisViewModel.mainTypeIndex
                                onActivated: root.analysisViewModel.mainTypeIndex = currentIndex
                            }
                        }

                        RowLayout {
                            visible: root.analysisViewModel.mainTypeIndex === 0
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
                                model: root.analysisViewModel.plotTypeOptions
                                textRole: "label"
                                currentIndex: root.analysisViewModel.plotSubtypeIndex
                                onActivated: root.analysisViewModel.plotSubtypeIndex = currentIndex
                            }
                        }
                    }
                }

                Controls.Panel {
                    visible: root.analysisViewModel.isEdit
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
                                model: root.analysisViewModel.exportFormatOptions
                                textRole: "label"
                                currentIndex: root.analysisViewModel.exportFormatIndex
                                onActivated: root.analysisViewModel.exportFormatIndex = currentIndex
                            }

                            Item {
                                Layout.fillWidth: true
                            }
                        }

                        Item {
                            id: includeAdjustmentsRow
                            objectName: "analysisIncludeAdjustmentsRow"
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.max(includeAdjustmentsCheckBox.implicitHeight, includeAdjustmentsLabel.implicitHeight)

                            RowLayout {
                                anchors.fill: parent

                                Controls.CheckBox {
                                    id: includeAdjustmentsCheckBox
                                    objectName: "analysisIncludeAdjustmentsCheckBox"
                                    Layout.fillWidth: false
                                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                    checked: root.analysisViewModel.includeAdjustments
                                }

                                Label {
                                    id: includeAdjustmentsLabel
                                    color: root.theme.textPrimary
                                    text: qsTr("Include adjustments")
                                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                }

                                Item {
                                    Layout.fillWidth: true
                                }
                            }

                            MouseArea {
                                objectName: "analysisIncludeAdjustmentsMouseArea"
                                anchors.fill: parent
                                z: 1
                                onClicked: root.analysisViewModel.includeAdjustments = !root.analysisViewModel.includeAdjustments
                            }
                        }
                    }
                }

                Controls.Panel {
                    visible: root.analysisViewModel.isEdit
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 0
                    Layout.preferredHeight: -1

                    Loader {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        sourceComponent: root.analysisViewModel.currentResultIsTable ? tableComp : plotComp
                    }
                }
            }
        }

        Controls.Panel {
            visible: !root.analysisViewModel.isEdit && root.analysisViewModel.filterEditMode
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight + root.theme.controlHeight + root.theme.spacingSmall * 3

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: root.analysisViewModel.filterContentIndex

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
                                analysisViewModel: root.analysisViewModel
                                Layout.fillWidth: true
                                Layout.preferredHeight: dateFilter.implicitHeight
                            }

                            Analysis.AnalysisPropertyFilter {
                                theme: root.theme
                                analysisViewModel: root.analysisViewModel
                                Layout.fillWidth: true
                            }

                            Analysis.AnalysisContractTypeFilter {
                                theme: root.theme
                                analysisViewModel: root.analysisViewModel
                                Layout.fillWidth: true
                            }

                            Analysis.AnalysisAllocatableFilter {
                                id: allocatableFilterPanel
                                theme: root.theme
                                analysisViewModel: root.analysisViewModel
                                mode: root.analysisViewModel.allocatableMode
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
                    analysisViewModel: root.analysisViewModel
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
            analysisViewModel: root.analysisViewModel
        }
    }

    Component {
        id: tableComp

        Analysis.AnalysisTableView {
            theme: root.theme
            analysisViewModel: root.analysisViewModel
        }
    }
}
