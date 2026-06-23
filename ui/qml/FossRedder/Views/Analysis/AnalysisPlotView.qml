/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisPlotView.qml
 * @brief Shows the rendered analysis preview image.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var analysisViewModel

    ColumnLayout {
        anchors.fill: root
        spacing: root.theme.spacingSmall

        Image {
            id: renderedPreviewImage
            objectName: "analysisPreviewImage"
            visible: root.analysisViewModel.renderedPreviewSource.length > 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: root.theme.chartPlotMinimumHeight
            Layout.preferredHeight: root.theme.chartPlotPreferredHeight
            source: root.analysisViewModel.renderedPreviewSource
            fillMode: Image.PreserveAspectFit
            horizontalAlignment: Image.AlignLeft
            verticalAlignment: Image.AlignTop
            smooth: true
            mipmap: true
            asynchronous: false
            cache: false
        }

        Rectangle {
            visible: root.analysisViewModel.renderedPreviewSource.length === 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: root.theme.chartPlotMinimumHeight
            color: root.theme.surfaceAlt
            border.width: root.theme.borderWidthThin
            border.color: root.theme.borderLight

            Label {
                anchors.centerIn: parent
                text: qsTr("No plot preview available")
                color: root.theme.textMuted
            }
        }
    }
}
