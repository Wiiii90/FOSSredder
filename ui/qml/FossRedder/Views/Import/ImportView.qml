/**
 * @file ui/qml/FossRedder/Views/Import/ImportView.qml
 * @brief Composes import workflow pages for statement import runs.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views.Import 1.0 as Import
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var importViewModel
    required property var theme

    Component.onCompleted: Qt.callLater(root.importViewModel.initializeImportView)

    StackLayout {
        objectName: "importContentStack"
        anchors.fill: parent
        anchors.margins: root.theme.pageContentMargin
        currentIndex: root.importViewModel.contentIndex

        Import.ImportHomeView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: root.theme
            importViewModel: root.importViewModel
        }

        Import.StatementDraftView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: root.theme
            importViewModel: root.importViewModel
        }
    }
}
