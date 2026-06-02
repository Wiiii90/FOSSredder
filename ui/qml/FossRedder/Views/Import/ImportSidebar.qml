/**
 * @file ui/qml/FossRedder/Views/Import/ImportSidebar.qml
 * @brief Provides the import log sidebar.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components

Item {
    id: root
    required property var importViewModel
    required property var theme

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
            model: root.importViewModel.importLogs
            selectedLogId: root.importViewModel.selectedDraftId
            onRunClicked: function (index, logId, draftAttached, statementId, draftId) {
                root.importViewModel.openImportLog(logId, draftAttached, statementId, draftId);
            }
            onDeleteClicked: function (index, logId, draftAttached, draftId) {
                root.importViewModel.deleteImportLog(logId, draftAttached, draftId);
            }
        }
    }
}
