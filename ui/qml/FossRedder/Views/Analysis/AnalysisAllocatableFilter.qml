/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisAllocatableFilter.qml
 * @brief Provides the AnalysisAllocatableFilter component.
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
    property string mode: "all"
    property bool initialized: false

    Layout.fillWidth: true
    contentSpacing: root.theme.spacingSmall

    RowLayout {
        Layout.fillWidth: true
        spacing: root.theme.spacingSmall

        Label {
            color: root.theme.textPrimary
            text: qsTr("Allocatable")
            Layout.preferredWidth: root.theme.formLabelWidth
        }

        Controls.DropdownMenu {
            id: allocatableCombo
            objectName: "analysisAllocatableModeComboBox"
            Layout.preferredWidth: root.theme.formFieldWidth
            model: [qsTr("All"), qsTr("Only allocatable"), qsTr("Only non allocatable")]
            currentIndex: root.mode === "allocatable" ? 1 : (root.mode === "non-allocatable" ? 2 : 0)
            onCurrentIndexChanged: {
                if (!root.initialized)
                    return;
                root.analysisViewModel.setAllocatableModeIndex(currentIndex);
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }

    Component.onCompleted: root.initialized = true
}
