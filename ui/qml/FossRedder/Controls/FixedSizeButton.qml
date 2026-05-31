/**
 * @file ui/qml/FossRedder/Controls/FixedSizeButton.qml
 * @brief Provides a fixed-size base for compact and navigation buttons.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.3

Button {
    id: root

    required property int buttonWidth
    required property int buttonHeight

    implicitWidth: root.buttonWidth
    implicitHeight: root.buttonHeight
    Layout.preferredWidth: root.buttonWidth
    Layout.minimumWidth: root.buttonWidth
    Layout.maximumWidth: root.buttonWidth
    Layout.preferredHeight: root.buttonHeight
    Layout.minimumHeight: root.buttonHeight
    Layout.maximumHeight: root.buttonHeight
}
