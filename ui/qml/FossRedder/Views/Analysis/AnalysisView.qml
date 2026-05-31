/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisView.qml
 * @brief Provides the AnalysisView composition.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views.Analysis 1.0 as Analysis
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var analysisState
    required property var theme

    onVisibleChanged: {
        if (visible)
            root.analysisState.refreshFromSelection()
    }

    ColumnLayout {
        anchors.fill: root
        spacing: root.theme.spacingSmall

        Analysis.AnalysisForm {
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: root.theme
            analysisState: root.analysisState
        }

        Analysis.AnalysisBottomBar {
            Layout.fillWidth: true
            Layout.leftMargin: root.theme.pageContentMargin
            Layout.rightMargin: root.theme.pageContentMargin
            Layout.bottomMargin: root.theme.pageContentMargin
            theme: root.theme
            analysisState: root.analysisState
        }
    }

    Component.onCompleted: root.analysisState.refreshFromSelection()
}
