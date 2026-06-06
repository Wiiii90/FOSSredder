/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftContractAllocatablePanel.qml
 * @brief Renders the allocatable toggle block embedded in the contract panel.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var importViewModel
    required property var theme

    readonly property int suggestionTone: root.importViewModel.suggestionTone(root.importViewModel.allocatableSuggestionConfidence)
    readonly property color suggestionColor: root.suggestionTone === 2 ? root.theme.successStrong : (root.suggestionTone === 1 ? root.theme.warning : root.theme.danger)

    Layout.fillHeight: false
    Layout.preferredHeight: implicitHeight
    implicitHeight: contentLayout.implicitHeight

    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Controls.Panel {
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.preferredHeight: implicitHeight
            background: Rectangle {
                radius: root.theme.radius
                color: root.theme.surfaceAlt
                border.width: 1
                border.color: root.suggestionColor
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                Label {
                    color: root.theme.textPrimary
                    text: qsTr("Select Allocatable")
                    Layout.fillWidth: true
                }

                Controls.Button {
                    objectName: "transactionDraftAllocatableToggle"
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.viewCompactActionButtonSize
                    fillColor: root.theme.surface
                    textColor: root.theme.textPrimary
                    bordered: true
                    filled: false
                    emphasized: false
                    text: root.importViewModel.effectiveAllocatable ? qsTr("Allocatable") : qsTr("Not allocatable")
                    onClicked: root.importViewModel.toggleAllocatable()
                }
            }
        }

        Label {
            objectName: "transactionDraftAllocatableSuggestionLabel"
            text: root.importViewModel.allocatableSuggestionText
            color: root.suggestionColor
            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.theme.spacingSmall
        }
    }
}
