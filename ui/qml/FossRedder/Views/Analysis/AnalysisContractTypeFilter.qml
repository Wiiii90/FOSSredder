/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisContractTypeFilter.qml
 * @brief Provides the AnalysisContractTypeFilter component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var analysisViewModel
    readonly property var selectedTypes: root.analysisViewModel.selectedContractTypes

    readonly property real actionButtonSize: root.theme.viewCompactActionButtonSize || root.theme.controlHeight || 32
    readonly property real actionButtonWidth: Math.max(root.actionButtonSize, 96)

    Layout.fillWidth: true
    Layout.fillHeight: false
    Layout.preferredHeight: implicitHeight
    contentSpacing: root.theme.spacingSmall

    background: Rectangle {
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: root.theme.borderWidthThin
        border.color: root.theme.border
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
        Layout.preferredHeight: implicitHeight
        spacing: root.theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true
            spacing: root.theme.spacingSmall

            Label {
                color: root.theme.textPrimary
                text: qsTr("Contract Types")
                Layout.fillWidth: true
            }

            RowLayout {
                spacing: root.theme.spacingSmall

                Controls.SecondaryButton {
                    objectName: "analysisContractTypeFilterAllButton"
                    text: qsTr("All")
                    Layout.preferredWidth: root.actionButtonWidth
                    Layout.preferredHeight: root.actionButtonSize
                    onClicked: root.analysisViewModel.selectAllContractTypes()
                }

                Controls.SecondaryButton {
                    objectName: "analysisContractTypeFilterNoneButton"
                    text: qsTr("None")
                    Layout.preferredWidth: root.actionButtonWidth
                    Layout.preferredHeight: root.actionButtonSize
                    onClicked: root.analysisViewModel.selectNoContractTypes()
                }
            }
        }

        Controls.CheckListPanel {
            Repeater {
                model: root.analysisViewModel.contractTypeRows

                delegate: RowLayout {
                    id: ctRow
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall
                    Layout.preferredHeight: Math.max(checkBox.implicitHeight, contractTypeLabel.implicitHeight)

                    Controls.CheckBox {
                        id: checkBox
                        objectName: "analysisContractTypeFilterCheckBox"
                        Layout.fillWidth: false
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        checked: root.selectedTypes.indexOf(String(ctRow.modelData.value)) !== -1
                        onClicked: root.analysisViewModel.setContractTypeSelected(String(ctRow.modelData.value), checked)
                    }

                    Label {
                        id: contractTypeLabel
                        color: root.theme.textPrimary
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        text: String(ctRow.modelData.label)
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignTop
                    }
                }
            }
        }
    }
}
