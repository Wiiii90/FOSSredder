/**
 * @file ui/qml/FossRedder/Views/Actor/ActorView.qml
 * @brief Provides the ActorView component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views.Actor 1.0 as Actor
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var actorViewModel
    required property var theme
    readonly property var actorRows: root.actorViewModel.actorRows
    readonly property bool isEdit: root.actorViewModel ? root.actorViewModel.isEdit : false
    readonly property bool hasChanges: root.actorViewModel ? root.actorViewModel.hasChanges : false
    readonly property string name: root.actorViewModel ? root.actorViewModel.name : ""
    readonly property var aliases: root.actorViewModel ? root.actorViewModel.aliases : []
    readonly property string aliasInputText: root.actorViewModel ? root.actorViewModel.aliasInputText : ""
    readonly property int aliasIndex: root.actorViewModel ? root.actorViewModel.aliasIndex : -1
    readonly property var selectedContractIds: root.actorViewModel ? root.actorViewModel.selectedContractIds : []

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Actor.ActorForm {
            Layout.fillWidth: true
            Layout.fillHeight: true
            actorViewModel: root.actorViewModel
            theme: root.theme
        }

        Actor.ActorBottomBar {
            Layout.fillWidth: true
            Layout.leftMargin: root.theme.pageContentMargin
            Layout.rightMargin: root.theme.pageContentMargin
            Layout.bottomMargin: root.theme.pageContentMargin
            theme: root.theme
            actorViewModel: root.actorViewModel
            actorRows: root.actorRows
        }
    }
}
