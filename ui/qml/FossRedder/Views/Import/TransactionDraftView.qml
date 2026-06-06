/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftView.qml
 * @brief Composes transaction draft review panels.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views.Import 1.0 as Import
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var importViewModel

    implicitHeight: layout.implicitHeight
    implicitWidth: layout.implicitWidth
    height: root.implicitHeight

    ColumnLayout {
        id: layout
        width: root.width
        spacing: root.theme.spacingSmall

        Import.TransactionDraftForm {
            Layout.fillWidth: true
            theme: root.theme
            importViewModel: root.importViewModel
        }

        Import.TransactionDraftProofPanel {
            importViewModel: root.importViewModel
            theme: root.theme
        }

        Import.TransactionDraftMetadataPanel {
            importViewModel: root.importViewModel
            theme: root.theme
        }

        Import.TransactionDraftContractPanel {
            Layout.fillWidth: true
            theme: root.theme
            importViewModel: root.importViewModel
        }
    }
}
