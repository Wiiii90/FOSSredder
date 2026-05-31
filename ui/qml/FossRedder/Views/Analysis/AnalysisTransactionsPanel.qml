/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisTransactionsPanel.qml
 * @brief Provides the AnalysisTransactionsPanel component.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var analysisState
    readonly property var txTheme: root.theme.analysis.transactions
    readonly property int txTableMinimumWidth: root.txTheme.applyColumnWidth + root.txTheme.statementColumnWidth + root.txTheme.transactionColumnWidth + root.txTheme.dateColumnWidth + root.txTheme.dateColumnWidth + root.txTheme.actorColumnWidth + root.txTheme.contractColumnWidth + root.txTheme.typeColumnWidth + root.txTheme.propertiesColumnWidth + root.txTheme.amountColumnWidth + root.theme.spacingSmall * root.txTheme.columnSpacingCount + root.theme.spacingSmall * root.txTheme.horizontalPaddingCount

    Layout.fillWidth: true
    Layout.fillHeight: true
    contentSpacing: root.theme.spacingSmall

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true
            Label {
                color: root.theme.textPrimary
                text: qsTr("Calc name")
                Layout.preferredWidth: root.theme.formLabelWidth
            }
            Controls.TextField {
                id: calcNameField
                objectName: "analysisCalcNameField"
                Layout.preferredWidth: root.theme.formFieldWidth
                placeholderText: qsTr("e.g. VAT")
                text: root.analysisState.calcName
                onTextChanged: root.analysisState.calcName = text
            }
            Label {
                color: root.theme.textPrimary
                text: qsTr("Calc %")
            }
            Controls.TextField {
                id: calcPercentField
                objectName: "analysisCalcPercentField"
                Layout.preferredWidth: root.txTheme.calcPercentFieldWidth
                placeholderText: qsTr("0")
                text: root.analysisState.calcPercentText
                onTextChanged: root.analysisState.calcPercentText = text
            }
            Controls.Button {
                objectName: "analysisApplyCalcButton"
                text: qsTr("Apply to selected")
                onClicked: root.analysisState.applySelectedCalc()
            }
        }

        Flickable {
            id: txViewport
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            flickableDirection: Flickable.HorizontalFlick
            contentWidth: txTable.width
            contentHeight: height
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.horizontal: Controls.AppScrollBar { hidden: true }

            Item {
                id: txTable
                width: Math.max(Math.max(0, txViewport.width - root.theme.scrollBarGutterWidth), root.txTableMinimumWidth)
                implicitHeight: headerRect.height + txList.contentHeight

                Rectangle {
                    id: headerRect
                    width: txTable.width
                    height: root.txTheme.headerHeight
                    color: root.theme.surfaceAlt
                    border.width: root.theme.borderWidthThin
                    border.color: root.theme.border

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: root.theme.spacingSmall
                        anchors.rightMargin: root.theme.spacingSmall
                        spacing: root.theme.spacingSmall
                        Label {
                            color: root.theme.textPrimary
                            text: qsTr("Apply")
                            Layout.preferredWidth: root.txTheme.applyColumnWidth
                        }
                        Label {
                            color: root.theme.textPrimary
                            text: qsTr("Statement")
                            Layout.preferredWidth: root.txTheme.statementColumnWidth
                        }
                        Label {
                            color: root.theme.textPrimary
                            text: qsTr("Transaction")
                            Layout.preferredWidth: root.txTheme.transactionColumnWidth
                        }
                        Label {
                            color: root.theme.textPrimary
                            text: qsTr("Booking Date")
                            Layout.preferredWidth: root.txTheme.dateColumnWidth
                        }
                        Label {
                            color: root.theme.textPrimary
                            text: qsTr("Valuta")
                            Layout.preferredWidth: root.txTheme.dateColumnWidth
                        }
                        Label {
                            color: root.theme.textPrimary
                            text: qsTr("Actor")
                            Layout.preferredWidth: root.txTheme.actorColumnWidth
                        }
                        Label {
                            color: root.theme.textPrimary
                            text: qsTr("Contract")
                            Layout.preferredWidth: root.txTheme.contractColumnWidth
                        }
                        Label {
                            color: root.theme.textPrimary
                            text: qsTr("Type")
                            Layout.preferredWidth: root.txTheme.typeColumnWidth
                        }
                        Label {
                            color: root.theme.textPrimary
                            text: qsTr("Properties")
                            Layout.preferredWidth: root.txTheme.propertiesColumnWidth
                            elide: Text.ElideRight
                        }
                        Label {
                            color: root.theme.textPrimary
                            text: qsTr("Amount")
                            Layout.preferredWidth: root.txTheme.amountColumnWidth
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }

                ListView {
                    id: txList
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: headerRect.bottom
                    height: Math.max(0, txViewport.height - headerRect.height)
                    clip: true
                    model: root.analysisState.previewTransactionRows
                    delegate: Rectangle {
                        id: txRow
                        required property var modelData
                        required property int index
                        width: txTable.width
                        height: root.txTheme.rowHeight
                        color: index % 2 === 0 ? root.theme.surface : root.theme.surfaceAlt

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: root.theme.spacingSmall
                            anchors.rightMargin: root.theme.spacingSmall
                            spacing: root.theme.spacingSmall

                            Controls.CheckBox {
                                objectName: "analysisTransactionSelectionCheckBox"
                                Layout.preferredWidth: root.txTheme.applyColumnWidth
                                Layout.fillWidth: false
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                checked: root.analysisState.selectedAdjustmentTxIds.indexOf(txRow.modelData.id) !== -1
                                onToggled: root.analysisState.setAdjustmentTransactionSelected(txRow.modelData.id, checked)
                            }

                            Label {
                                color: root.theme.textPrimary
                                text: txRow.modelData.statementName
                                Layout.preferredWidth: root.txTheme.statementColumnWidth
                                elide: Text.ElideRight
                            }
                            Label {
                                color: root.theme.textPrimary
                                text: txRow.modelData.transactionName
                                Layout.preferredWidth: root.txTheme.transactionColumnWidth
                                elide: Text.ElideRight
                            }
                            Label {
                                color: root.theme.textPrimary
                                text: txRow.modelData.date
                                Layout.preferredWidth: root.txTheme.dateColumnWidth
                                elide: Text.ElideRight
                            }
                            Label {
                                color: root.theme.textPrimary
                                text: txRow.modelData.valuta
                                Layout.preferredWidth: root.txTheme.dateColumnWidth
                                elide: Text.ElideRight
                            }
                            Label {
                                color: root.theme.textPrimary
                                text: txRow.modelData.actorName
                                Layout.preferredWidth: root.txTheme.actorColumnWidth
                                elide: Text.ElideRight
                            }
                            Label {
                                color: root.theme.textPrimary
                                text: txRow.modelData.contractName
                                Layout.preferredWidth: root.txTheme.contractColumnWidth
                                elide: Text.ElideRight
                            }
                            Label {
                                color: root.theme.textPrimary
                                text: txRow.modelData.contractType
                                Layout.preferredWidth: root.txTheme.typeColumnWidth
                                elide: Text.ElideRight
                            }
                            Label {
                                color: root.theme.textPrimary
                                text: txRow.modelData.propertiesLabel
                                Layout.preferredWidth: root.txTheme.propertiesColumnWidth
                                elide: Text.ElideRight
                            }
                            Label {
                                color: root.theme.textPrimary
                                Layout.preferredWidth: root.txTheme.amountColumnWidth
                                horizontalAlignment: Text.AlignRight
                                text: txRow.modelData.amountText
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: root.theme.borderWidthThin
            color: root.theme.border
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                color: root.theme.textPrimary
                text: root.analysisState.previewStatementCountText
                Layout.preferredWidth: root.txTheme.metricsStatementWidth
            }
            Label {
                color: root.theme.textPrimary
                text: root.analysisState.previewTransactionCountText
                Layout.preferredWidth: root.txTheme.metricsTransactionWidth
            }
            Item {
                Layout.fillWidth: true
            }
            Label {
                color: root.theme.textPrimary
                text: root.analysisState.previewAmountSumText
                Layout.preferredWidth: root.txTheme.metricsAmountWidth
                horizontalAlignment: Text.AlignRight
            }
        }
    }
}
