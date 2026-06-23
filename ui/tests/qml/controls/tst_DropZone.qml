/**
 * @file ui/tests/qml/controls/tst_DropZone.qml
 * @brief Provides QML tests for DropZone behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Controls 1.0

import "../common"

TestCase {
    id: testCase
    name: "DropZoneTests"
    when: windowShown
    width: 480
    height: 320

    Component {
        id: dropZoneComponent
        DropZone {
            width: 360
            height: 180
            title: "Drop files"
            fileSummary: "2 files"
            queuedCount: 2
        }
    }

    function createControl() {
        return createTemporaryObject(dropZoneComponent, testCase)
    }

    function test_CTRL_DZ_001_clickRequestsBrowse() {
        const control = createControl()
        var requested = false
        control.browseRequested.connect(function() { requested = true })

        const clickArea = Lookup.findObject(control, "dropZoneMouseArea")
        verify(clickArea !== null)
        clickArea.clicked(null)

        compare(requested, true)
    }

    function test_CTRL_DZ_002_publicStateIsApplied() {
        const control = createControl()

        compare(control.title, "Drop files")
        compare(control.fileSummary, "2 files")
        compare(control.queuedCount, 2)
    }
}
