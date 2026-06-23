/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftFieldRow.qml
 * @brief Provides a reusable two-column field row layout for transaction draft panels.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: root
    required property var theme

    property string leftLabel: ""
    property string rightLabel: ""
    property real leftWeight: 1
    property real rightWeight: 1
    property real columnSpacing: root.theme.spacingMedium
    property Component leftContent
    property Component rightContent

    implicitWidth: row.implicitWidth
    implicitHeight: Math.max(leftColumn.implicitHeight, rightColumn.implicitHeight)

    RowLayout {
        id: row
        anchors.fill: parent
        spacing: root.columnSpacing

        ColumnLayout {
            id: leftColumn
            Layout.fillWidth: true
            Layout.preferredWidth: root.leftWeight
            Layout.alignment: Qt.AlignTop
            spacing: root.theme.spacingSmall

            Label {
                color: root.theme.textPrimary
                text: root.leftLabel
                Layout.fillWidth: true
            }

            Loader {
                id: leftLoader
                Layout.fillWidth: true
                Layout.preferredHeight: Math.max(root.theme.controlHeight, implicitHeight)
                Layout.alignment: Qt.AlignTop
                sourceComponent: root.leftContent
            }
        }

        ColumnLayout {
            id: rightColumn
            Layout.fillWidth: true
            Layout.preferredWidth: root.rightWeight
            Layout.alignment: Qt.AlignTop
            spacing: root.theme.spacingSmall

            Label {
                color: root.theme.textPrimary
                text: root.rightLabel
                Layout.fillWidth: true
            }

            Loader {
                id: rightLoader
                Layout.fillWidth: true
                Layout.preferredHeight: Math.max(root.theme.controlHeight, implicitHeight)
                Layout.alignment: Qt.AlignTop
                sourceComponent: root.rightContent
            }
        }
    }
}
