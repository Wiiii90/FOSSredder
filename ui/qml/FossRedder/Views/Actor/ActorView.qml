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
    required property var actorState
    required property var theme
    readonly property var actorRows: root.actorState.actorRows
    readonly property bool isEdit: root.actorState ? root.actorState.isEdit : false
    readonly property bool hasChanges: root.actorState ? root.actorState.hasChanges : false
    readonly property string name: root.actorState ? root.actorState.name : ""
    readonly property var aliases: root.actorState ? root.actorState.aliases : []
    readonly property string aliasInputText: root.actorState ? root.actorState.aliasInputText : ""
    readonly property int aliasIndex: root.actorState ? root.actorState.aliasIndex : -1
    readonly property var selectedContractIds: root.actorState ? root.actorState.selectedContractIds : []

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Actor.ActorForm {
            Layout.fillWidth: true
            Layout.fillHeight: true
            actorState: root.actorState
            theme: root.theme
        }

        Actor.ActorBottomBar {
            Layout.fillWidth: true
            Layout.leftMargin: root.theme.pageContentMargin
            Layout.rightMargin: root.theme.pageContentMargin
            Layout.bottomMargin: root.theme.pageContentMargin
            theme: root.theme
            actorState: root.actorState
            actorRows: root.actorRows
        }
    }
}
