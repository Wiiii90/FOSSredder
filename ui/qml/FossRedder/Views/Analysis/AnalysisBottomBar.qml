/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisBottomBar.qml
 * @brief Provides the Analysis view action bar.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var analysisViewModel

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: root
        theme: root.theme

        Controls.PrevButton {
            objectName: "analysisPreviousButton"
            enabled: root.analysisViewModel.hasRows
            onClicked: root.analysisViewModel.navigate(-1)
        }

        Item { Layout.fillWidth: true }

        Controls.SecondaryButton {
            objectName: "analysisToggleFilterContentButton"
            visible: !root.analysisViewModel.isEdit && root.analysisViewModel.filterEditMode
            text: "⇆"
            Layout.preferredWidth: 48
            onClicked: root.analysisViewModel.toggleFilterContent()
        }

        Controls.DangerButton {
            objectName: "analysisResetButton"
            visible: !root.analysisViewModel.isEdit
            text: qsTr("Reset")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.analysisViewModel.clearFilters()
        }

        Controls.SuccessButton {
            objectName: "analysisCreateButton"
            visible: !root.analysisViewModel.isEdit
            text: qsTr("Create")
            enabled: root.analysisViewModel.canSubmit
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.analysisViewModel.submitCreate()
        }

        Controls.DangerButton {
            objectName: "analysisDeleteButton"
            visible: root.analysisViewModel.isEdit
            text: qsTr("Delete")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.analysisViewModel.deleteCurrent()
        }

        Controls.SuccessButton {
            objectName: "analysisUpdateButton"
            visible: root.analysisViewModel.isEdit
            text: qsTr("Update")
            enabled: root.analysisViewModel.canSubmit
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.analysisViewModel.submitUpdate()
        }

        Item { Layout.fillWidth: true }

        Controls.NextButton {
            objectName: "analysisNextButton"
            enabled: root.analysisViewModel.hasRows
            onClicked: root.analysisViewModel.navigate(1)
        }
    }
}
