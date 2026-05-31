/**
 * @file ui/qml/FossRedder/Views/Booking/BookingSidebar.qml
 * @brief Provides the BookingSidebar component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var theme
    required property var bookingState
    readonly property real rowRadius: root.theme.viewSidebarRowRadius
    readonly property real rowHeight: root.theme.viewSidebarRowHeight
    readonly property color panelSurfaceAlt: root.theme.surfaceAlt

    ColumnLayout {
        anchors.fill: root
        spacing: root.theme.spacingSmall

        Flickable {
            id: bookingStatementsFlick
            objectName: "bookingStatementsFlick"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: statementColumn.implicitHeight

            ScrollBar.vertical: Controls.AppScrollBar {
                parent: bookingStatementsFlick
                anchors.right: bookingStatementsFlick.right
                anchors.rightMargin: root.theme.viewSidebarScrollBarOuterInset
                anchors.top: bookingStatementsFlick.top
                anchors.bottom: bookingStatementsFlick.bottom
                persistent: true
            }

            Column {
                id: statementColumn
                x: root.theme.viewSidebarEntryInset
                width: Math.max(0, parent.width - root.theme.viewSidebarEntryInsetTotal)
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.bookingState.statementRows

                    delegate: Rectangle {
                        id: statementEntry
                        objectName: "bookingStatementRow_" + statementEntry.statementId
                        required property var modelData
                        width: statementColumn.width
                        property bool collapsed: false
                        property string statementId: statementEntry.modelData.id
                        property string statementName: statementEntry.modelData.name
                        readonly property bool isSelectedStatement: statementEntry.statementId === root.bookingState.selectedStatementId
                        color: statementMouseArea.containsMouse ? root.theme.sidebarHoverFill : root.panelSurfaceAlt
                        radius: root.rowRadius
                        border.width: root.theme.borderWidthThin
                        border.color: statementEntry.isSelectedStatement ? root.theme.selectionBorder : (statementMouseArea.containsMouse ? root.theme.sidebarHoverBorder : root.theme.borderSoft)
                        implicitHeight: statementContent.implicitHeight + (root.theme.spacingSmall * 2)

                        Column {
                            id: statementContent
                            anchors.fill: statementEntry
                            anchors.margins: root.theme.spacingSmall
                            spacing: root.theme.spacingSmall

                            Item {
                                id: statementHeader
                                width: statementContent.width
                                height: Math.max(root.rowHeight - (root.theme.spacingSmall * 2), statementNameText.implicitHeight + (root.theme.margins * 2), collapseButton.implicitHeight + (root.theme.margins * 2))

                                MouseArea {
                                    id: statementMouseArea
                                    objectName: "bookingStatementMouse_" + statementEntry.statementId
                                    anchors.fill: statementHeader
                                    acceptedButtons: Qt.LeftButton
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    preventStealing: true
                                    onClicked: root.bookingState.selectStatement(statementEntry.statementId)
                                }

                                Text {
                                    id: statementNameText
                                    anchors.left: statementHeader.left
                                    anchors.right: collapseButton.left
                                    anchors.rightMargin: root.theme.spacingSmall
                                    anchors.top: statementHeader.top
                                    anchors.topMargin: root.theme.margins
                                    text: statementEntry.statementName
                                    color: root.theme.textPrimary
                                    elide: Text.ElideRight
                                }

                                Controls.DisclosureButton {
                                    id: collapseButton
                                    anchors.right: statementHeader.right
                                    anchors.top: statementHeader.top
                                    anchors.topMargin: root.theme.margins
                                    anchors.rightMargin: root.theme.margins
                                    implicitWidth: root.theme.spacingLarge + root.theme.margins * 4
                                    implicitHeight: root.theme.spacingLarge + root.theme.margins * 4
                                    expanded: !statementEntry.collapsed
                                    onClicked: statementEntry.collapsed = !statementEntry.collapsed
                                }
                            }

                            Column {
                                id: transactionColumn
                                width: statementContent.width
                                spacing: root.theme.spacingSmall
                                visible: !statementEntry.collapsed

                                Repeater {
                                    model: statementEntry.modelData.transactions

                                    delegate: Rectangle {
                                        id: transactionEntry
                                        objectName: "bookingTransactionRow_" + transactionEntry.transactionId
                                        required property var modelData
                                        property string transactionId: transactionEntry.modelData.id
                                        width: transactionColumn.width
                                        height: root.rowHeight
                                        radius: root.rowRadius
                                        color: transactionEntry.transactionId === root.bookingState.selectedTransactionId ? root.theme.selectionHighlight : (transactionMouse.containsMouse ? root.theme.sidebarHoverFill : "transparent")
                                        border.color: transactionEntry.transactionId === root.bookingState.selectedTransactionId ? root.theme.selectionBorder : (transactionMouse.containsMouse ? root.theme.sidebarHoverBorder : root.theme.borderSoft)
                                        border.width: root.theme.borderWidthThin

                                        MouseArea {
                                            id: transactionMouse
                                            objectName: "bookingTransactionMouse_" + transactionEntry.transactionId
                                            anchors.fill: transactionEntry
                                            acceptedButtons: Qt.LeftButton
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            preventStealing: true
                                            onClicked: root.bookingState.selectTransaction(statementEntry.statementId, transactionEntry.transactionId)
                                        }

                                        RowLayout {
                                            anchors.fill: transactionEntry
                                            anchors.margins: root.theme.spacingSmall
                                            spacing: root.theme.spacingSmall

                                            Text {
                                                Layout.fillWidth: true
                                                text: transactionEntry.modelData.name
                                                color: root.theme.textPrimary
                                                elide: Text.ElideRight
                                            }

                                            Text {
                                                text: transactionEntry.modelData.bookingDate
                                                color: root.theme.textMuted
                                                elide: Text.ElideRight
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
