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
    required property var importState
    required property var theme

    Component.onCompleted: Qt.callLater(root.importState.activatePage)

    StackLayout {
        objectName: "importContentStack"
        anchors.fill: parent
        anchors.margins: root.theme.pageContentMargin
        currentIndex: root.importState.contentIndex

        Import.ImportHomeView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: root.theme
            importState: root.importState
        }

        Import.StatementDraftView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: root.theme
            importState: root.importState
            draft: root.importState.currentDraft
        }
    }
}
