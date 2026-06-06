/**
 * @file ui/qml/FossRedder/Components/StatusChip.qml
 * @brief Provides a compact themed status label.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    required property var theme
    property string text: ""
    property string tone: "neutral"

    readonly property bool successTone: root.tone === "success"
    readonly property bool warningTone: root.tone === "warning"
    readonly property bool dangerTone: root.tone === "danger"
    readonly property bool infoTone: root.tone === "info"
    readonly property color toneFill: root.successTone ? root.theme.statusSuccessFill : (root.warningTone ? root.theme.statusWarningFill : (root.dangerTone ? root.theme.statusDangerFill : (root.infoTone ? root.theme.statusInfoFill : root.theme.statusNeutralFill)))
    readonly property color toneText: root.successTone ? root.theme.statusSuccessText : (root.warningTone ? root.theme.statusWarningText : (root.dangerTone ? root.theme.statusDangerText : (root.infoTone ? root.theme.statusInfoText : root.theme.statusNeutralText)))
    readonly property color toneBorder: root.successTone ? root.theme.statusSuccessBorder : (root.warningTone ? root.theme.statusWarningBorder : (root.dangerTone ? root.theme.statusDangerBorder : (root.infoTone ? root.theme.statusInfoBorder : root.theme.statusNeutralBorder)))

    implicitWidth: statusText.implicitWidth + root.theme.spacingSmall * 2
    implicitHeight: Math.max(root.theme.viewAliasChipHeight - root.theme.spacingSmall, statusText.implicitHeight + root.theme.margins * 2)

    Rectangle {
        anchors.fill: parent
        radius: root.theme.viewAliasChipRadius
        color: root.toneFill
        border.width: root.theme.borderWidthThin
        border.color: root.toneBorder
    }

    Label {
        id: statusText
        anchors.fill: parent
        anchors.leftMargin: root.theme.spacingSmall
        anchors.rightMargin: root.theme.spacingSmall
        text: root.text
        color: root.toneText
        font.family: root.theme.fontFamily
        font.pointSize: root.theme.fontSizeSmall
        font.weight: Font.Medium
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
