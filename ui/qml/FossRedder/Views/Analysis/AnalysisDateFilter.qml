/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisDateFilter.qml
 * @brief Provides the AnalysisDateFilter component.
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
    property bool initialized: false
    readonly property real modeFieldWidth: root.theme.formFieldWidth
    readonly property real valueFieldWidth: root.theme.formFieldWidth
    readonly property real yearFieldWidth: (root.theme.formFieldWidth * 2) + root.theme.spacingSmall + implicitFromLabel.implicitWidth + implicitToLabel.implicitWidth

    Layout.fillWidth: true
    contentSpacing: root.theme.spacingSmall

    RowLayout {
        Layout.fillWidth: true
        spacing: root.theme.spacingSmall

        Controls.DropdownMenu {
            id: dateFieldCombo
            objectName: "analysisDateFieldComboBox"
            Layout.preferredWidth: root.theme.formFieldWidth
            model: [qsTr("Booking Date"), qsTr("Valuta")]
            currentIndex: root.analysisState.dateFieldIndex
            onCurrentIndexChanged: {
                if (root.initialized)
                    root.analysisState.dateFieldIndex = currentIndex;
            }
        }

        Controls.DropdownMenu {
            id: dateModeCombo
            objectName: "analysisDateModeComboBox"
            Layout.preferredWidth: root.theme.formFieldWidth
            model: [qsTr("Year"), qsTr("Date Range")]
            currentIndex: root.analysisState.dateModeIndex
            onCurrentIndexChanged: {
                if (root.initialized)
                    root.analysisState.dateModeIndex = currentIndex;
            }
        }

        Label {
            id: implicitFromLabel
            color: root.theme.textPrimary
            visible: root.analysisState.dateModeIndex === 1
            text: qsTr("From")
        }

        Controls.TextField {
            id: dateFromField
            objectName: "analysisDateFromField"
            visible: root.analysisState.dateModeIndex === 1
            Layout.preferredWidth: root.theme.formFieldWidth
            placeholderText: qsTr("YYYY-MM-DD")
            text: root.analysisState.dateFromValue
            onTextChanged: {
                if (root.initialized)
                    root.analysisState.dateFromValue = text;
            }
        }

        Label {
            id: implicitToLabel
            color: root.theme.textPrimary
            visible: root.analysisState.dateModeIndex === 1
            text: qsTr("To")
        }

        Controls.TextField {
            id: dateToField
            objectName: "analysisDateToField"
            visible: root.analysisState.dateModeIndex === 1
            Layout.preferredWidth: root.theme.formFieldWidth
            placeholderText: qsTr("YYYY-MM-DD")
            text: root.analysisState.dateToValue
            onTextChanged: {
                if (root.initialized)
                    root.analysisState.dateToValue = text;
            }
        }

        Controls.TextField {
            id: yearField
            objectName: "analysisYearField"
            visible: root.analysisState.dateModeIndex === 0
            Layout.preferredWidth: root.yearFieldWidth
            placeholderText: qsTr("YYYY")
            text: root.analysisState.yearValue
            onTextChanged: {
                if (root.initialized)
                    root.analysisState.yearValue = text;
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }

    Component.onCompleted: root.initialized = true
}
