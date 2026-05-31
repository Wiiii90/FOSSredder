/**
 * @file ui/qml/FossRedder/Controls/CheckBox.qml
 * @brief Provides the CheckBox component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
pragma ComponentBehavior: Bound

CheckBox {
    id: control
    Layout.fillWidth: true
    Layout.alignment: Qt.AlignVCenter

    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize

    rightPadding: Theme.spacingSmall
}
