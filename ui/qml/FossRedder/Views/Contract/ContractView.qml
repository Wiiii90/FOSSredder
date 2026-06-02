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
    required property var contractViewModel
    required property var theme
    readonly property var contractRows: root.contractViewModel.contractRows
    readonly property bool isEdit: root.contractViewModel ? root.contractViewModel.isEdit : false
    readonly property bool hasChanges: root.contractViewModel ? root.contractViewModel.hasChanges : false
    readonly property string name: root.contractViewModel ? root.contractViewModel.name : ""
    readonly property string type: root.contractViewModel ? root.contractViewModel.type : ""
    readonly property var aliases: root.contractViewModel ? root.contractViewModel.aliases : []
    readonly property string aliasInputText: root.contractViewModel ? root.contractViewModel.aliasInputText : ""
    readonly property int aliasIndex: root.contractViewModel ? root.contractViewModel.aliasIndex : -1
    readonly property var selectedActorIds: root.contractViewModel ? root.contractViewModel.selectedActorIds : []
    readonly property var selectedPropertyIds: root.contractViewModel ? root.contractViewModel.selectedPropertyIds : []

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Contract.ContractForm {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contractViewModel: root.contractViewModel
            theme: root.theme
        }

        Contract.ContractBottomBar {
            Layout.fillWidth: true
            Layout.leftMargin: root.theme.pageContentMargin
            Layout.rightMargin: root.theme.pageContentMargin
            Layout.bottomMargin: root.theme.pageContentMargin
            theme: root.theme
            contractViewModel: root.contractViewModel
            contractRows: root.contractRows
        }
    }
}
