/**
 * @file ui/qml/FossRedder/Controls/CompactRemoveButton.qml
 * @brief Provides a shared compact remove action button.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
pragma ComponentBehavior: Bound

SecondaryButton {
    text: qsTr("-")
    textColor: Theme.textMuted
    implicitWidth: Theme.viewCompactActionButtonSize
    implicitHeight: Theme.viewCompactActionButtonSize
    Layout.preferredWidth: Theme.viewCompactActionButtonSize
    Layout.minimumWidth: Theme.viewCompactActionButtonSize
    Layout.maximumWidth: Theme.viewCompactActionButtonSize
    Layout.preferredHeight: Theme.viewCompactActionButtonSize
    Layout.minimumHeight: Theme.viewCompactActionButtonSize
    Layout.maximumHeight: Theme.viewCompactActionButtonSize

    Accessible.name: qsTr("Remove")
}
