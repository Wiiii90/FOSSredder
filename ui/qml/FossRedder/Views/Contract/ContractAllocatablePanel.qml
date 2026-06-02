/**
 * @file ui/qml/FossRedder/Views/Contract/ContractAllocatablePanel.qml
 * @brief Provides the ContractAllocatablePanel component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var contractViewModel

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
            text: qsTr("Allocatable")
            Layout.fillWidth: true
        }

        Controls.DropdownMenu {
            id: allocatableModeCombo
            objectName: "contractAllocatableModeCombo"
            Layout.fillWidth: true
            model: [qsTr("Mixed"), qsTr("Always allocatable"), qsTr("Never allocatable")]
            currentIndex: root.contractViewModel && root.contractViewModel.allocatableMode === "allocatable" ? 1 : (root.contractViewModel && root.contractViewModel.allocatableMode === "non-allocatable" ? 2 : 0)
            onActivated: function (index) {
                if (root.contractViewModel)
                    root.contractViewModel.allocatableMode = index === 1 ? "allocatable" : (index === 2 ? "non-allocatable" : "mixed");
            }
        }
    }
}
