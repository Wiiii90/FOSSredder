/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisSidebar.qml
 * @brief Provides the Analysis sidebar list.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var theme
    required property var analysisViewModel
    width: root.theme.shellSidebarPreferredWidth

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Flickable {
            id: analysisSidebarFlick
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: analysisColumn.implicitHeight

            ScrollBar.vertical: Controls.AppScrollBar {
                parent: analysisSidebarFlick
                anchors.right: analysisSidebarFlick.right
                anchors.rightMargin: root.theme.viewSidebarScrollBarOuterInset
                anchors.top: analysisSidebarFlick.top
                anchors.bottom: analysisSidebarFlick.bottom
                persistent: true
            }

            Column {
                id: analysisColumn
                x: root.theme.viewSidebarEntryInset
                width: Math.max(0, parent.width - root.theme.viewSidebarEntryInsetTotal)
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.analysisViewModel.analysisRows

                    delegate: Rectangle {
                        id: analysisRow
                        objectName: "analysisSidebarRow"
                        required property var modelData
                        width: analysisColumn.width
                        height: root.theme.viewSidebarRowHeight
                        radius: root.theme.viewSidebarRowRadius
                        color: analysisRow.modelData.id === root.analysisViewModel.selectedAnalysisId ? root.theme.selectionHighlight : (analysisMouse.containsMouse ? root.theme.sidebarHoverFill : "transparent")
                        border.color: analysisRow.modelData.id === root.analysisViewModel.selectedAnalysisId ? root.theme.selectionBorder : (analysisMouse.containsMouse ? root.theme.sidebarHoverBorder : root.theme.borderSoft)
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            id: analysisMouse
                            objectName: "analysisSidebarRowMouseArea"
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.analysisViewModel.selectAnalysis(analysisRow.modelData.id)
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: root.theme.viewSidebarRowSpacing

                            Text {
                                width: parent.width
                                text: analysisRow.modelData.name
                                color: root.theme.textPrimary
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: analysisRow.modelData.type
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
