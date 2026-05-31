/**
 * @file ui/qml/FossRedder/Controls/CheckListPanel.qml
 * @brief Provides a nested surface for checkbox selection lists.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Frame {
    id: root

    property int contentSpacing: Theme.spacingSmall
    readonly property real panelAvailableWidth: parent ? parent.width : Theme.checkListPanelMinWidth
    readonly property real contentPreferredWidth: body.implicitWidth + Theme.scrollBarGutterWidth
    readonly property real contentPreferredHeight: body.implicitHeight
    readonly property real maxContentHeight: (Theme.checkboxSize + Theme.spacingSmall) * Theme.checkListPanelMaxVisibleRows
    readonly property real targetWidth: Math.min(root.panelAvailableWidth, Math.max(Theme.checkListPanelMinWidth, root.contentPreferredWidth + (root.padding * 2)))
    readonly property real targetHeight: Math.min(root.contentPreferredHeight, root.maxContentHeight) + (root.padding * 2)

    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    Layout.fillWidth: false
    Layout.fillHeight: false
    Layout.preferredWidth: root.targetWidth
    Layout.maximumWidth: root.panelAvailableWidth
    Layout.preferredHeight: root.targetHeight
    padding: Theme.panelPadding

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surface
        border.width: Theme.borderWidthThin
        border.color: Theme.border
    }

    contentItem: Flickable {
        id: viewport
        implicitWidth: body.implicitWidth + Theme.scrollBarGutterWidth
        implicitHeight: Math.min(body.implicitHeight, root.maxContentHeight)
        clip: true
        contentWidth: viewport.width
        contentHeight: body.height

        ScrollBar.vertical: AppScrollBar {}

        ColumnLayout {
            id: body
            width: Math.max(implicitWidth, viewport.width - Theme.scrollBarGutterWidth)
            height: implicitHeight
            spacing: root.contentSpacing
        }
    }

    default property alias items: body.data
}
