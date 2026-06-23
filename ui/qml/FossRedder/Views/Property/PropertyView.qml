/**
 * @file ui/qml/FossRedder/Views/Property/PropertyView.qml
 * @brief Provides the PropertyView component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views.Property 1.0 as Property
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var propertyViewModel
    required property var theme

    readonly property var propertyRows: root.propertyViewModel.propertyRows
    readonly property bool isEdit: root.propertyViewModel ? root.propertyViewModel.isEdit : false
    readonly property bool hasChanges: root.propertyViewModel ? root.propertyViewModel.hasChanges : false
    readonly property string name: root.propertyViewModel ? root.propertyViewModel.name : ""
    readonly property var aliases: root.propertyViewModel ? root.propertyViewModel.aliases : []
    readonly property string aliasInputText: root.propertyViewModel ? root.propertyViewModel.aliasInputText : ""
    readonly property int aliasIndex: root.propertyViewModel ? root.propertyViewModel.aliasIndex : -1
    readonly property var selectedContractIds: root.propertyViewModel ? root.propertyViewModel.selectedContractIds : []

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Property.PropertyForm {
            Layout.fillWidth: true
            Layout.fillHeight: true
            propertyViewModel: root.propertyViewModel
            theme: root.theme
        }

        Property.PropertyBottomBar {
            Layout.fillWidth: true
            Layout.leftMargin: root.theme.pageContentMargin
            Layout.rightMargin: root.theme.pageContentMargin
            Layout.bottomMargin: root.theme.pageContentMargin
            theme: root.theme
            propertyViewModel: root.propertyViewModel
            propertyRows: root.propertyRows
        }
    }
}
