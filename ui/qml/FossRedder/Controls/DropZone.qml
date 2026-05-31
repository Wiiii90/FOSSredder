/**
 * @file ui/qml/FossRedder/Controls/DropZone.qml
 * @brief Provides the DropZone component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0 as FR
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    readonly property var theme: FR.Theme
    implicitHeight: Math.max(dropZoneMinimumHeight, dropZoneRect.implicitHeight)
    property int dropZoneMinimumHeight: 140

    property string title: qsTr("Drop PDFs here")
    property string subtitle: qsTr("")

    property bool allowBrowse: true
    property string browseText: qsTr("Browse...")
    property bool clickToBrowse: true

    property var files: []
    property string fileSummary: ""
    property int queuedCount: 0

    signal browseRequested()

    Rectangle {
        id: dropZoneRect
        anchors.fill: parent
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: root.theme.borderWidthThin
        border.color: root.theme.border

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: root.theme.spacing
            spacing: root.theme.spacingSmall

            Image {
                Layout.alignment: Qt.AlignHCenter
                source: "../assets/import.svg"
                Layout.preferredWidth: root.theme.viewSectionIconSize
                Layout.preferredHeight: root.theme.viewSectionIconSize
                fillMode: Image.PreserveAspectFit
                smooth: true
                opacity: 0.85
            }

            Label {
                Layout.fillWidth: true
                text: root.title
                font.bold: true
                color: root.theme.textPrimary
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Label {
                Layout.fillWidth: true
                text: root.subtitle
                color: root.theme.textMuted
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                visible: root.subtitle && root.subtitle.length > 0
            }

            Label {
                Layout.fillWidth: true
                visible: root.files && root.files.length > 0
                text: root.fileSummary
                color: root.theme.textMuted
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Label {
                Layout.fillWidth: true
                visible: root.queuedCount > 0
                text: qsTr("Queue: %1").arg(root.queuedCount)
                color: root.theme.textMuted
                horizontalAlignment: Text.AlignHCenter
            }

            Item { Layout.fillHeight: true }

            Controls.SecondaryButton {
                visible: root.allowBrowse
                Layout.alignment: Qt.AlignHCenter
                text: root.browseText
                onClicked: root.browseRequested()
            }
        }
    }

    MouseArea {
        objectName: "dropZoneMouseArea"
        anchors.fill: parent
        enabled: root.clickToBrowse && root.enabled
        cursorShape: Qt.PointingHandCursor
        onClicked: root.browseRequested()
    }
}
