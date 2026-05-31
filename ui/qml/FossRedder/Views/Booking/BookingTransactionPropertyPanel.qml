/**
 * @file ui/qml/FossRedder/Views/Booking/BookingTransactionPropertyPanel.qml
 * @brief Provides the BookingTransactionPropertyPanel component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var bookingState
    readonly property var selectedPropertyIds: root.bookingState.selectedPropertyIds

    Layout.fillWidth: true
    Layout.fillHeight: false
    Layout.preferredWidth: 1
    Layout.preferredHeight: implicitHeight
    contentSpacing: root.theme.spacingSmall

    background: Rectangle {
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: 1
        border.color: root.theme.border
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
        Layout.preferredHeight: implicitHeight
        spacing: root.theme.spacingSmall

        Label {
            color: root.theme.textPrimary
            text: qsTr("Property")
            Layout.fillWidth: true
        }

        Controls.CheckListPanel {
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.preferredHeight: implicitHeight
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.bookingState.propertyRows

                    delegate: RowLayout {
                        id: propertyDelegate
                        required property var modelData
                        readonly property string propertyId: propertyDelegate.modelData.id
                        readonly property string propertyLabel: propertyDelegate.modelData.display

                        Layout.fillWidth: true
                        spacing: root.theme.spacingSmall

                        Controls.CheckBox {
                            objectName: "bookingTransactionPropertyCheckBox"
                            Layout.fillWidth: false
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            checked: root.selectedPropertyIds.indexOf(propertyDelegate.propertyId) !== -1
                            onToggled: root.bookingState.setPropertySelected(propertyDelegate.propertyId, checked)
                        }

                        Label {
                            color: root.theme.textPrimary
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            text: propertyDelegate.propertyLabel
                            elide: Text.ElideRight
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
