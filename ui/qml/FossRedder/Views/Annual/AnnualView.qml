/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualView.qml
 * @brief Provides the Annual view composition.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views.Annual 1.0 as Annual
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var annualViewModel
    required property var theme

    onVisibleChanged: {
        if (visible)
            root.annualViewModel.refreshFromSelection()
    }

    ColumnLayout {
        anchors.fill: root
        spacing: root.theme.spacingSmall

        Annual.AnnualForm {
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: root.theme
            annualViewModel: root.annualViewModel
        }

        Annual.AnnualBottomBar {
            Layout.fillWidth: true
            Layout.leftMargin: root.theme.pageContentMargin
            Layout.rightMargin: root.theme.pageContentMargin
            Layout.bottomMargin: root.theme.pageContentMargin
            theme: root.theme
            annualViewModel: root.annualViewModel
        }
    }
}
