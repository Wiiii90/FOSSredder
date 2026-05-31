/**
 * @file ui/qml/FossRedder/Views/Contract/ContractView.qml
 * @brief Provides the ContractView component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views.Contract 1.0 as Contract
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var contractState
    required property var theme
    readonly property var contractRows: root.contractState.contractRows
    readonly property bool isEdit: root.contractState ? root.contractState.isEdit : false
    readonly property bool hasChanges: root.contractState ? root.contractState.hasChanges : false
    readonly property string name: root.contractState ? root.contractState.name : ""
    readonly property string type: root.contractState ? root.contractState.type : ""
    readonly property var aliases: root.contractState ? root.contractState.aliases : []
    readonly property string aliasInputText: root.contractState ? root.contractState.aliasInputText : ""
    readonly property int aliasIndex: root.contractState ? root.contractState.aliasIndex : -1
    readonly property var selectedActorIds: root.contractState ? root.contractState.selectedActorIds : []
    readonly property var selectedPropertyIds: root.contractState ? root.contractState.selectedPropertyIds : []

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Contract.ContractForm {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contractState: root.contractState
            theme: root.theme
        }

        Contract.ContractBottomBar {
            Layout.fillWidth: true
            Layout.leftMargin: root.theme.pageContentMargin
            Layout.rightMargin: root.theme.pageContentMargin
            Layout.bottomMargin: root.theme.pageContentMargin
            theme: root.theme
            contractState: root.contractState
            contractRows: root.contractRows
        }
    }
}
