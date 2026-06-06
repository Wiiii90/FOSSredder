/**
 * @file ui/qml/FossRedder/Controls/DropdownMenu.qml
 * @brief Provides the DropdownMenu component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

ComboBox {
    id: control
    Layout.fillWidth: true
    Layout.preferredHeight: Theme.controlHeight
    implicitHeight: Theme.controlHeight
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize
    palette.text: Theme.textPrimary
    palette.buttonText: Theme.textPrimary
    palette.highlight: Theme.primary
    palette.highlightedText: Theme.onPrimary
    leftPadding: Theme.controlPaddingHorizontal
    rightPadding: indicator.width + Theme.controlPaddingHorizontal
    topPadding: Theme.controlPaddingVertical
    bottomPadding: Theme.controlPaddingVertical

    function itemText(itemModel) {
        if (!itemModel)
            return "";
        if (control.textRole && itemModel[control.textRole] !== undefined)
            return String(itemModel[control.textRole]);
        if (itemModel.display !== undefined)
            return String(itemModel.display);
        if (itemModel.modelData !== undefined)
            return String(itemModel.modelData);
        return String(itemModel);
    }

    contentItem: Text {
        leftPadding: Theme.controlPaddingHorizontal
        rightPadding: control.indicator.width + Theme.controlPaddingHorizontal
        text: control.displayText
        font.family: Theme.fontFamily
        font.pointSize: Theme.fontSize
        color: control.enabled ? Theme.textPrimary : Theme.textMuted
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Item {
        implicitHeight: Theme.controlHeight
        anchors.fill: parent

        Rectangle {
            anchors.fill: parent
            y: control.activeFocus ? 1 : 2
            radius: Theme.radius
            color: Theme.shadow
            opacity: control.enabled ? Theme.controlShadowOpacity : 0.02
        }

        Rectangle {
            anchors.fill: parent
            radius: Theme.radius
            color: control.hovered || control.activeFocus || control.popup.visible ? Theme.controlHoverFill : Theme.controlFill
            border.color: control.activeFocus || control.popup.visible ? Theme.controlFocusBorder : (control.hovered ? Theme.controlHoverBorder : Theme.borderMedium)
            border.width: Theme.borderWidthThin

            Behavior on color {
                ColorAnimation {
                    duration: Theme.animationDurationFast
                }
            }

            Behavior on border.color {
                ColorAnimation {
                    duration: Theme.animationDurationFast
                }
            }
        }
    }

    delegate: ItemDelegate {
        id: optionDelegate
        required property int index
        required property var model
        width: control.width
        implicitHeight: Theme.menuItemHeight
        highlighted: control.highlightedIndex === index
        enabled: model.available !== false
        hoverEnabled: true
        opacity: enabled ? 1.0 : 0.5

        contentItem: Text {
            leftPadding: Theme.controlPaddingHorizontal
            rightPadding: Theme.controlPaddingHorizontal
            text: control.itemText(optionDelegate.model)
            font.family: Theme.fontFamily
            font.pointSize: Theme.fontSize
            color: optionDelegate.enabled ? Theme.textPrimary : Theme.textMuted
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            color: optionDelegate.highlighted || optionDelegate.hovered ? Theme.menuHoverFill : Theme.menuFill
            Behavior on color {
                ColorAnimation {
                    duration: Theme.animationDurationFast
                }
            }
        }
    }

    popup: Popup {
        id: popup
        y: control.height
        width: control.width
        z: Theme.popupZ
        padding: 0
        modal: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        palette.text: Theme.textPrimary
        palette.windowText: Theme.textPrimary
        palette.buttonText: Theme.textPrimary
        palette.highlight: Theme.primary
        palette.highlightedText: Theme.onPrimary

        background: Item {
            Rectangle {
                anchors.fill: parent
                y: 3
                radius: Theme.radius
                color: Theme.shadow
                opacity: Theme.popupShadowOpacity
            }

            Rectangle {
                anchors.fill: parent
                radius: Theme.radius
                color: Theme.menuFill
                border.width: Theme.borderWidthThin
                border.color: Theme.borderMedium
            }
        }

        contentItem: ListView {
            implicitHeight: Math.min(contentHeight, Theme.dropdownPopupMaxHeight)
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            clip: true

            ScrollBar.vertical: AppScrollBar {}
        }
    }

    indicator: Rectangle {
        width: Theme.dropdownIndicatorWidth
        height: parent.height
        color: "transparent"
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        Text {
            anchors.centerIn: parent
            text: "▾"
            color: Theme.textPrimary
            font.family: Theme.fontFamily
            font.pointSize: Theme.fontSize
        }
    }
}
