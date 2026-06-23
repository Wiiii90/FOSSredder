/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualSidebar.qml
 * @brief Provides the Annual sidebar list.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var annualViewModel
    required property var theme
    width: root.theme.shellSidebarPreferredWidth

    ColumnLayout {
        anchors.fill: root
        spacing: root.theme.spacingSmall

        Flickable {
            id: annualSidebarFlick
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: annualColumn.implicitHeight

            ScrollBar.vertical: Controls.AppScrollBar {
                parent: annualSidebarFlick
                anchors.right: annualSidebarFlick.right
                anchors.rightMargin: root.theme.viewSidebarScrollBarOuterInset
                anchors.top: annualSidebarFlick.top
                anchors.bottom: annualSidebarFlick.bottom
                persistent: true
            }

            Column {
                id: annualColumn
                x: root.theme.viewSidebarEntryInset
                width: Math.max(0, parent.width - root.theme.viewSidebarEntryInsetTotal)
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.annualViewModel.annualRows

                    delegate: Rectangle {
                        id: annualRow
                        objectName: "annualSidebarRow"
                        required property var modelData
                        width: annualColumn.width
                        height: root.theme.viewSidebarRowHeight
                        radius: root.theme.viewSidebarRowRadius
                        color: annualRow.modelData.id === root.annualViewModel.selectedAnnualId ? root.theme.selectionHighlight : (annualMouse.containsMouse ? root.theme.sidebarHoverFill : "transparent")
                        border.color: annualRow.modelData.id === root.annualViewModel.selectedAnnualId ? root.theme.selectionBorder : (annualMouse.containsMouse ? root.theme.sidebarHoverBorder : root.theme.borderSoft)
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            id: annualMouse
                            objectName: "annualSidebarRowMouseArea"
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.annualViewModel.selectAnnual(annualRow.modelData.id)
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: root.theme.viewSidebarRowSpacing

                            Text {
                                width: parent.width
                                text: annualRow.modelData.display
                                color: root.theme.textPrimary
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: String(annualRow.modelData.year)
                                color: root.theme.textMuted
                                elide: Text.ElideRight
                                visible: text.length > 0
                            }
                        }
                    }
                }
            }
        }
    }
}
