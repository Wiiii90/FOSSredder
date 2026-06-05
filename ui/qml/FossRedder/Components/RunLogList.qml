/**
 * @file ui/qml/FossRedder/Components/RunLogList.qml
 * @brief Provides the RunLogList component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var theme
    property var model: null
    property string selectedLogId: ""
    property int cardMinHeight: 0
    property real cardRadius: -1
    property int cardPadding: root.theme.spacingSmall
    property int listTopMargin: root.theme.spacingSmall
    property int itemSpacing: root.theme.spacingSmall
    property color baseBorderColor: root.theme.borderSoft
    property color hoverBorderColor: root.theme.sidebarHoverBorder
    property int actionButtonSize: root.theme.viewCompactActionButtonSize
    property int headerTopInset: 0
    property bool fileRowsClickable: false
    signal runClicked(int index, string logId, bool draftAttached, string statementId, string draftId)
    signal deleteClicked(int index, string logId, bool draftAttached, string draftId)

    readonly property int detailLineHeight: detailLineMetric.height

    TextMetrics {
        id: detailLineMetric
        text: "Mg"
    }

    ListView {
        id: runsList
        objectName: "runLogList"
        anchors.fill: parent
        clip: true
        topMargin: root.listTopMargin
        spacing: root.itemSpacing
        model: root.model

        ScrollBar.vertical: Controls.AppScrollBar {
            parent: runsList
            anchors.right: runsList.right
            anchors.rightMargin: root.theme.viewSidebarScrollBarOuterInset
            anchors.top: runsList.top
            anchors.bottom: runsList.bottom
            persistent: true
        }

        delegate: Item {
            id: runDelegate
            required property int index
            required property string logId
            required property var time
            required property var status
            required property var file
            required property var message
            required property string displayTime
            required property string displayTitle
            required property string displayStatusDetail
            required property bool draftAttached
            required property string draftId
            required property string statementId
            readonly property string normalizedStatus: String(status || "").toLowerCase()
            readonly property bool navigableDraft: draftAttached || draftId.length > 0 || normalizedStatus === "draft"
            readonly property bool navigableStatement: statementId.length > 0
            readonly property bool openableFile: root.fileRowsClickable && String(file || "").length > 0
            readonly property bool selected: root.selectedLogId.length > 0 && (root.selectedLogId === runDelegate.logId || root.selectedLogId === runDelegate.draftId)
            width: runsList.width
            height: runEntry.height

            Rectangle {
                id: runEntry
                objectName: "runLogCard_" + runDelegate.logId
                x: root.theme.viewSidebarEntryInset
                width: Math.max(0, runDelegate.width - root.theme.viewSidebarEntryInsetTotal)
                height: Math.max(root.cardMinHeight, content.implicitHeight + root.cardPadding)
                radius: root.cardRadius >= 0 ? root.cardRadius : root.theme.radius
                color: runDelegate.selected ? root.theme.selectionHighlight : (rowClickArea.containsMouse ? root.theme.sidebarHoverFill : "transparent")
                border.width: root.theme.borderWidthThin
                border.color: runDelegate.selected ? root.theme.selectionBorder : (rowClickArea.containsMouse ? root.hoverBorderColor : root.baseBorderColor)

                ColumnLayout {
                    id: content
                    z: 1
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: root.cardPadding
                    anchors.topMargin: root.cardPadding
                    anchors.rightMargin: root.cardPadding
                    anchors.bottomMargin: root.cardPadding
                    spacing: root.theme.spacingSmall

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: root.actionButtonSize
                        Layout.topMargin: root.headerTopInset

                        Label {
                            color: root.theme.textPrimary
                            objectName: "runLogTime_" + runDelegate.logId
                            text: runDelegate.displayTime
                            font.pointSize: root.theme.fontSizeSmall
                            opacity: 0.7
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter
                            verticalAlignment: Text.AlignVCenter
                            elide: Label.ElideRight
                        }
                        Label {
                            visible: false
                            objectName: "runLogStatus_" + runDelegate.logId
                            text: runDelegate.status
                        }

                        StatusChip {
                            objectName: "runLogStatusChip_" + runDelegate.logId
                            text: runDelegate.status
                            theme: root.theme
                            tone: runDelegate.normalizedStatus === "success" || runDelegate.normalizedStatus === "finalized" ? "success" : (runDelegate.normalizedStatus === "running" || runDelegate.normalizedStatus === "paused" || runDelegate.normalizedStatus === "draft" ? "warning" : "danger")
                            Layout.alignment: Qt.AlignVCenter
                        }

                        Controls.SecondaryButton {
                            z: 3
                            objectName: "runLogDelete_" + runDelegate.logId
                            text: "×"
                            implicitHeight: root.actionButtonSize
                            implicitWidth: root.actionButtonSize
                            Layout.alignment: Qt.AlignTop | Qt.AlignRight
                            textColor: root.theme.textMuted
                            onClicked: root.deleteClicked(runDelegate.index, runDelegate.logId, runDelegate.draftAttached, runDelegate.draftId)
                        }
                    }

                    Label {
                        color: root.theme.textPrimary
                        objectName: "runLogTitle_" + runDelegate.logId
                        text: runDelegate.displayTitle
                        Layout.fillWidth: true
                        elide: Label.ElideRight
                    }

                    Label {
                        objectName: "runLogDetail_" + runDelegate.logId
                        Layout.fillWidth: true
                        Layout.preferredHeight: root.detailLineHeight
                        Layout.minimumHeight: root.detailLineHeight
                        text: runDelegate.displayStatusDetail
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        opacity: text.length > 0 ? 0.8 : 0
                        color: root.theme.textMuted
                    }
                }

                MouseArea {
                    id: rowClickArea
                    objectName: "runLogRow_" + runDelegate.logId
                    z: 2
                    anchors.fill: parent
                    anchors.rightMargin: root.actionButtonSize + root.theme.spacingSmall
                    hoverEnabled: true
                    preventStealing: true
                    enabled: runDelegate.navigableDraft || runDelegate.navigableStatement || runDelegate.openableFile
                    cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: root.runClicked(runDelegate.index, runDelegate.logId, runDelegate.navigableDraft, runDelegate.statementId, runDelegate.draftId)
                }
            }
        }
    }
}
