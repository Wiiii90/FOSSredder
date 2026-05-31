/**
 * @file ui/qml/FossRedder/Views/Import/ImportSidebar.qml
 * @brief Provides the import run sidebar.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components

Item {
    id: root
    required property var importState
    required property var theme

    Component.onCompleted: root.importState.refreshFromWorkspace()

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Components.RunLogList {
            theme: root.theme
            Layout.fillWidth: true
            Layout.fillHeight: true
            cardMinHeight: root.theme.viewSidebarRowHeight + root.theme.spacingSmall
            cardRadius: root.theme.viewSidebarRowRadius
            cardPadding: root.theme.spacingSmall
            listTopMargin: 0
            itemSpacing: root.theme.spacingSmall
            baseBorderColor: root.theme.borderSoft
            actionButtonSize: root.theme.viewCompactActionButtonSizeTiny
            headerTopInset: root.theme.spacingSmall
            model: root.importState.runModel
            selectedLogId: root.importState.selectedRunLogId
            onRunClicked: function (index, logId, draftAttached, statementId, draftId) {
                root.importState.activateRun(index, logId, draftAttached, statementId, draftId);
            }
            onDeleteClicked: function (index, draftAttached, draftId) {
                root.importState.deleteRun(index, draftAttached, draftId);
            }
        }
    }
}
