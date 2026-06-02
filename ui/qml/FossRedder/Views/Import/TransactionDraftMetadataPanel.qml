/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftMetadataPanel.qml
 * @brief Displays metadata preview fields for the current transaction draft.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var transactionViewModel

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    contentSpacing: root.theme.spacingSmall

    background: Rectangle {
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: 1
        border.color: root.theme.border
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        Label {
            color: root.theme.textPrimary
            text: qsTr("Metadata")
            Layout.fillWidth: true
        }
        Controls.TextArea {
            objectName: "transactionDraftMetadataTextArea"
            Layout.fillWidth: true
            Layout.preferredHeight: implicitHeight
            Layout.minimumHeight: 0
            readOnly: true
            placeholderText: qsTr("Extracted metadata")
            text: root.transactionViewModel.metadataText
        }
    }
}
