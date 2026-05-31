/**
 * @file ui/qml/FossRedder/Controls/Panel.qml
 * @brief Provides the Panel component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
pragma ComponentBehavior: Bound

Frame {
    id: root

    property int contentSpacing: Theme.spacingSmall

    padding: Theme.panelPadding

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surface
        border.width: 1
        border.color: Theme.border
    }

    contentItem: ColumnLayout {
        id: body
        spacing: root.contentSpacing
    }

    default property alias content: body.data
}
