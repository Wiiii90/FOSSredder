/**
 * @file ui/qml/FossRedder/Views/Export/ExportSidebar.qml
 * @brief Provides the Export sidebar.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components

Item {
    id: root
    required property var exportViewModel
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
            model: root.exportViewModel.exportLogs
            fileRowsClickable: true
            onRunClicked: function (_index, logId, _draftAttached, _statementId) {
                root.exportViewModel.openExportLogLocation(logId);
            }
            onDeleteClicked: function (_index, logId, _draftAttached, _draftId) {
                root.exportViewModel.deleteExportLog(logId);
            }
        }
    }
}
