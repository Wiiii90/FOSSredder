/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualForm.qml
 * @brief Provides the Annual form surface.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Annual 1.0 as Annual

Item {
    id: root
    required property var theme
    required property var annualState

    ColumnLayout {
        anchors.fill: root
        anchors.margins: root.theme.pageContentMargin
        anchors.bottomMargin: 0
        spacing: root.theme.spacingSmall

        Flickable {
            id: annualFormScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: annualWorkspaceContent.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Controls.AppScrollBar { hidden: true }

            ColumnLayout {
                id: annualWorkspaceContent
                width: annualFormScroll.width
                height: Math.max(implicitHeight, annualFormScroll.height)
                spacing: root.theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        color: root.theme.textPrimary
                        text: qsTr("Annual Name")
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }

                    Controls.TextField {
                        objectName: "annualNameField"
                        Layout.fillWidth: true
                        text: root.annualState.name
                        onTextChanged: root.annualState.name = text
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        color: root.theme.textPrimary
                        text: qsTr("Year")
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: root.theme.spacingSmall

                        Controls.SecondaryButton {
                            objectName: "annualYearDecreaseButton"
                            Layout.preferredWidth: root.theme.controlHeight
                            Layout.preferredHeight: root.theme.controlHeight
                            Layout.minimumWidth: root.theme.controlHeight
                            Layout.maximumWidth: root.theme.controlHeight
                            text: "\u25BC"
                            onClicked: root.annualState.stepYear(-1)
                        }

                        Rectangle {
                            objectName: "annualYearField"
                            Layout.fillWidth: true
                            Layout.preferredHeight: root.theme.controlHeight
                            radius: root.theme.radius
                            color: root.theme.surface
                            border.width: root.theme.borderWidthThin
                            border.color: root.theme.border

                            Label {
                                anchors.fill: parent
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                text: String(root.annualState.year)
                                color: root.theme.textPrimary
                            }
                        }

                        Controls.SecondaryButton {
                            objectName: "annualYearIncreaseButton"
                            Layout.preferredWidth: root.theme.controlHeight
                            Layout.preferredHeight: root.theme.controlHeight
                            Layout.minimumWidth: root.theme.controlHeight
                            Layout.maximumWidth: root.theme.controlHeight
                            text: "\u25B2"
                            onClicked: root.annualState.stepYear(1)
                        }
                    }
                }

                StackLayout {
                    id: annualWorkspaceStack
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: root.annualState.workspaceIndex

                    Annual.AnnualAnalysesPanel {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.bottomMargin: root.theme.spacingSmall
                        theme: root.theme
                        annualState: root.annualState
                    }

                    Annual.AnnualTransactionsPanel {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        theme: root.theme
                        annualState: root.annualState
                    }
                }

                Annual.AnnualVerificationPanel {
                    Layout.fillWidth: true
                    theme: root.theme
                    annualState: root.annualState
                }
            }
        }
    }
}
