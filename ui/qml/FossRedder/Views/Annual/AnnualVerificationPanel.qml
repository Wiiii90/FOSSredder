/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualVerificationPanel.qml
 * @brief Provides the Annual verification summary panel.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    objectName: "annualVerificationPanel"
    required property var theme
    required property var annualState

    Layout.fillWidth: true
    contentSpacing: root.theme.spacingSmall

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        Label {
            color: root.theme.textPrimary
            text: qsTr("Statistics")
            Layout.fillWidth: true
        }

        Controls.Panel {
            Layout.fillWidth: true
            contentSpacing: root.theme.spacingSmall

            background: Rectangle {
                radius: root.theme.radius
                color: root.theme.surfaceAlt
                border.width: root.theme.borderWidthThin
                border.color: root.theme.border
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.annualState.verificationRows

                    delegate: RowLayout {
                        id: verificationRow
                        required property var modelData
                        Layout.fillWidth: true
                        spacing: root.theme.spacingSmall

                        Label {
                            text: verificationRow.modelData.label
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            color: verificationRow.modelData.tone === "success" ? root.theme.success : verificationRow.modelData.tone === "danger" ? root.theme.danger : root.theme.textPrimary
                        }

                        Label {
                            Layout.minimumWidth: root.theme.formLabelWidth / 2
                            horizontalAlignment: Text.AlignRight
                            text: String(verificationRow.modelData.value)
                            color: verificationRow.modelData.tone === "success" ? root.theme.success : verificationRow.modelData.tone === "danger" ? root.theme.danger : root.theme.textPrimary
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall

                    Label {
                        color: root.theme.textPrimary
                        text: qsTr("Statuses")
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        color: root.theme.textPrimary
                        objectName: "annualStatusSummaryLabel"
                        text: root.annualState.statusSummaryText
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }
    }
}
