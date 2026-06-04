/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftProofPanel.qml
 * @brief Renders proof image for validating the current transaction draft.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var importViewModel

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

        Image {
            objectName: "transactionDraftProofImage"
            Layout.fillWidth: true
            Layout.preferredHeight: source.toString().length > 0 ? Math.max(180, paintedHeight > 0 ? paintedHeight : width * 0.6) : 0
            Layout.minimumHeight: 0
            visible: source.toString().length > 0
            fillMode: Image.PreserveAspectFit
            source: root.importViewModel.proofSource
            cache: true
            asynchronous: true
        }
    }
}
