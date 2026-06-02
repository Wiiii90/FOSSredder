/**
 * @file ui/tests/qml/views/annual/tst_AnnualVerificationPanel.qml
 * @brief Provides QML tests for AnnualVerificationPanel rendering.
 */

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Annual 1.0 as Annual
import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "AnnualVerificationPanelTests"
    when: windowShown
    width: 640
    height: 320

    property var annualViewModel: QtObject {
        property var verificationRows: [
            { label: "Included transactions", value: 2, tone: "primary" },
            { label: "Transactions from a different year than selected", value: 0, tone: "success" }
        ]
        property string statusSummaryText: "Neutral: 0, Unverified: 1, Verified: 1, Completed: 0"
    }

    property var theme: ({
        spacingSmall: 8,
        radius: 4,
        surfaceAlt: "#f5f5f5",
        borderWidthThin: 1,
        border: "#cccccc",
        formLabelWidth: 160,
        success: "#008f5a",
        danger: "#b00020",
        textPrimary: "#202020"
    })

    Component {
        id: panelComponent
        Annual.AnnualVerificationPanel {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            annualViewModel: testCase.annualViewModel
        }
    }

    function createPanel() {
        return createTemporaryObject(panelComponent, testCase)
    }

    function test_ANN_VP_001_verificationSummaryRendersAnnualStateRows() {
        const panel = createPanel()

        verify(TestSupport.findRequired(Lookup, panel, "annualVerificationPanel") !== null)
        compare(TestSupport.findRequired(Lookup, panel, "annualStatusSummaryLabel").text,
                "Neutral: 0, Unverified: 1, Verified: 1, Completed: 0")
    }
}
