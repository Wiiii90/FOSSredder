/**
 * @file ui/tests/qml/views/booking/tst_BookingTransactionPropertyPanel.qml
 * @brief Provides QML tests for BookingTransactionPropertyPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Booking 1.0 as Booking

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "BookingTransactionPropertyPanelTests"
    when: windowShown
    width: 360
    height: 160

    property string lastPropertyId: ""
    property bool lastPropertySelected: false
    property int setPropertyCalls: 0

    property var theme: QtObject {
        property int spacingSmall: 6
        property int radius: 3
        property int borderWidthThin: 1
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
    }

    property var bookingViewModel: QtObject {
        property var propertyRows: [{ id: "property-1", display: "Flat 1" }]
        property var selectedPropertyIds: ["property-1"]
        function isPropertySelected(propertyId) {
            return selectedPropertyIds.indexOf(propertyId) !== -1
        }
        function setPropertySelected(propertyId, selected) {
            testCase.setPropertyCalls += 1
            testCase.lastPropertyId = propertyId
            testCase.lastPropertySelected = selected
            selectedPropertyIds = selected ? [propertyId] : []
        }
    }

    Component {
        id: panelComponent
        Booking.BookingTransactionPropertyPanel {
            width: testCase.width
            height: testCase.height
            theme: testCase.theme
            bookingViewModel: testCase.bookingViewModel
        }
    }

    function init() {
        lastPropertyId = ""
        lastPropertySelected = false
        setPropertyCalls = 0
        bookingViewModel.selectedPropertyIds = ["property-1"]
    }

    function findRequired(root, objectName) {
        const found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function test_BKG_TP_001_propertySelectionCallsBookingState() {
        const panel = createTemporaryObject(panelComponent, testCase)
        const checkBox = findRequired(panel, "bookingTransactionPropertyCheckBox")

        compare(checkBox.checked, true)
        checkBox.checked = false
        checkBox.toggled()

        compare(lastPropertyId, "property-1")
        compare(lastPropertySelected, false)
        compare(setPropertyCalls, 1)
    }

    function test_BKG_TP_002_checkboxRebindsWhenSelectedPropertyIdsChange() {
        const panel = createTemporaryObject(panelComponent, testCase)
        const checkBox = findRequired(panel, "bookingTransactionPropertyCheckBox")

        compare(checkBox.checked, true)
        bookingViewModel.selectedPropertyIds = []
        wait(50)

        compare(checkBox.checked, false)
        compare(setPropertyCalls, 0)
    }
}
