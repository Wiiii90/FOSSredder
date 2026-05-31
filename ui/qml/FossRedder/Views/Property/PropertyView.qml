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
    required property var propertyState
    required property var theme

    readonly property var propertyRows: root.propertyState.propertyRows
    readonly property bool isEdit: root.propertyState ? root.propertyState.isEdit : false
    readonly property bool hasChanges: root.propertyState ? root.propertyState.hasChanges : false
    readonly property string name: root.propertyState ? root.propertyState.name : ""
    readonly property var aliases: root.propertyState ? root.propertyState.aliases : []
    readonly property string aliasInputText: root.propertyState ? root.propertyState.aliasInputText : ""
    readonly property int aliasIndex: root.propertyState ? root.propertyState.aliasIndex : -1
    readonly property var selectedContractIds: root.propertyState ? root.propertyState.selectedContractIds : []

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Property.PropertyForm {
            Layout.fillWidth: true
            Layout.fillHeight: true
            propertyState: root.propertyState
            theme: root.theme
        }

        Property.PropertyBottomBar {
            Layout.fillWidth: true
            Layout.leftMargin: root.theme.pageContentMargin
            Layout.rightMargin: root.theme.pageContentMargin
            Layout.bottomMargin: root.theme.pageContentMargin
            theme: root.theme
            propertyState: root.propertyState
            propertyRows: root.propertyRows
        }
    }
}
