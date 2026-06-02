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
    required property var analysisViewModel
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
            currentIndex: root.analysisViewModel.dateFieldIndex
            onCurrentIndexChanged: {
                if (root.initialized)
                    root.analysisViewModel.dateFieldIndex = currentIndex;
            }
        }

        Controls.DropdownMenu {
            id: dateModeCombo
            objectName: "analysisDateModeComboBox"
            Layout.preferredWidth: root.theme.formFieldWidth
            model: [qsTr("Year"), qsTr("Date Range")]
            currentIndex: root.analysisViewModel.dateModeIndex
            onCurrentIndexChanged: {
                if (root.initialized)
                    root.analysisViewModel.dateModeIndex = currentIndex;
            }
        }

        Label {
            id: implicitFromLabel
            color: root.theme.textPrimary
            visible: root.analysisViewModel.dateModeIndex === 1
            text: qsTr("From")
        }

        Controls.TextField {
            id: dateFromField
            objectName: "analysisDateFromField"
            visible: root.analysisViewModel.dateModeIndex === 1
            Layout.preferredWidth: root.theme.formFieldWidth
            placeholderText: qsTr("YYYY-MM-DD")
            text: root.analysisViewModel.dateFromValue
            onTextChanged: {
                if (root.initialized)
                    root.analysisViewModel.dateFromValue = text;
            }
        }

        Label {
            id: implicitToLabel
            color: root.theme.textPrimary
            visible: root.analysisViewModel.dateModeIndex === 1
            text: qsTr("To")
        }

        Controls.TextField {
            id: dateToField
            objectName: "analysisDateToField"
            visible: root.analysisViewModel.dateModeIndex === 1
            Layout.preferredWidth: root.theme.formFieldWidth
            placeholderText: qsTr("YYYY-MM-DD")
            text: root.analysisViewModel.dateToValue
            onTextChanged: {
                if (root.initialized)
                    root.analysisViewModel.dateToValue = text;
            }
        }

        Controls.TextField {
            id: yearField
            objectName: "analysisYearField"
            visible: root.analysisViewModel.dateModeIndex === 0
            Layout.preferredWidth: root.yearFieldWidth
            placeholderText: qsTr("YYYY")
            text: root.analysisViewModel.yearValue
            onTextChanged: {
                if (root.initialized)
                    root.analysisViewModel.yearValue = text;
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }

    Component.onCompleted: root.initialized = true
}
