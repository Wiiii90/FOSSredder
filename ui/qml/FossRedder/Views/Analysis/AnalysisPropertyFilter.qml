/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisPropertyFilter.qml
 * @brief Provides the AnalysisPropertyFilter component.
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
    readonly property var selectedIds: root.analysisViewModel.selectedPropertyIds

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
                text: qsTr("Properties")
                Layout.fillWidth: true
            }

            RowLayout {
                spacing: root.theme.spacingSmall

                Controls.SecondaryButton {
                    objectName: "analysisPropertyFilterAllButton"
                    text: qsTr("All")
                    Layout.preferredWidth: root.actionButtonWidth
                    Layout.preferredHeight: root.actionButtonSize
                    onClicked: root.analysisViewModel.selectAllProperties()
                }

                Controls.SecondaryButton {
                    objectName: "analysisPropertyFilterNoneButton"
                    text: qsTr("None")
                    Layout.preferredWidth: root.actionButtonWidth
                    Layout.preferredHeight: root.actionButtonSize
                    onClicked: root.analysisViewModel.selectNoProperties()
                }
            }
        }

        Controls.CheckListPanel {
            Repeater {
                model: root.analysisViewModel.propertyFilterRows

                delegate: RowLayout {
                    id: rowRoot
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall
                    Layout.preferredHeight: Math.max(checkBox.implicitHeight, propertyLabel.implicitHeight)

                    Controls.CheckBox {
                        id: checkBox
                        objectName: "analysisPropertyFilterCheckBox"
                        Layout.fillWidth: false
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        checked: root.selectedIds.indexOf(rowRoot.modelData.id) !== -1
                        onClicked: root.analysisViewModel.setPropertySelected(rowRoot.modelData.id, checked)
                    }

                    Label {
                        id: propertyLabel
                        color: root.theme.textPrimary
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        text: rowRoot.modelData.name
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignTop
                    }
                }
            }
        }
    }
}
