/**
 * @file ui/qml/FossRedder/Views/Import/ImportForm.qml
 * @brief Provides the ImportForm component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var importViewModel

    contentSpacing: root.theme.spacingSmall

    RowLayout {
        Layout.fillWidth: true
        Label {
            color: root.theme.textPrimary
            text: qsTr("Source")
            Layout.preferredWidth: root.theme.formLabelWidth
        }
        Controls.DropdownMenu {
            id: sourceKind
            objectName: "importSourceComboBox"
            model: root.importViewModel.importSourceLabels
            currentIndex: 0
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Label {
            color: root.theme.textPrimary
            text: qsTr("Strategy")
            Layout.preferredWidth: root.theme.formLabelWidth
        }
        Controls.DropdownMenu {
            id: strategy
            objectName: "importStrategyComboBox"
            model: root.importViewModel.statementStrategyLabels
            currentIndex: 0
        }
    }
}
