/**
 * @file ui/qml/FossRedder/Views/Contract/ContractTypePanel.qml
 * @brief Provides the ContractTypePanel component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var contractState

    Layout.fillWidth: true
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
            text: qsTr("Type")
            Layout.fillWidth: true
        }

        Controls.TextField {
            objectName: "contractTypeField"
            Layout.fillWidth: true
            text: root.contractState ? root.contractState.type : ""
            onTextChanged: if (root.contractState)
                root.contractState.type = text
            onTextEdited: if (root.contractState)
                root.contractState.type = text
        }
    }
}
