/**
 * @file ui/tests/qml/controls/tst_FixedSizeButtons.qml
 * @brief Provides QML tests for fixed-size public button variants.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0

TestCase {
    id: testCase
    name: "FixedSizeButtonTests"
    when: windowShown
    width: 480
    height: 320

    Component {
        id: compactAddComponent
        CompactAddButton {}
    }

    Component {
        id: compactRemoveComponent
        CompactRemoveButton {}
    }

    Component {
        id: disclosureComponent
        DisclosureButton {}
    }

    Component {
        id: nextComponent
        NextButton {}
    }

    Component {
        id: previousPageComponent
        PrevPageButton {}
    }

    function test_CTRL_FSB_001_compactButtonsUseThemeSize() {
        const addButton = createTemporaryObject(compactAddComponent, testCase)
        const removeButton = createTemporaryObject(compactRemoveComponent, testCase)

        compare(addButton.implicitWidth, Theme.viewCompactActionButtonSize)
        compare(addButton.implicitHeight, Theme.viewCompactActionButtonSize)
        compare(removeButton.implicitWidth, Theme.viewCompactActionButtonSize)
        compare(removeButton.implicitHeight, Theme.viewCompactActionButtonSize)
    }

    function test_CTRL_FSB_002_navigationButtonsUseThemeSize() {
        const nextButton = createTemporaryObject(nextComponent, testCase)
        const previousPageButton = createTemporaryObject(previousPageComponent, testCase)

        compare(nextButton.implicitWidth, Theme.viewNavigationButtonWidth)
        compare(nextButton.implicitHeight, Theme.buttonMinHeight)
        compare(previousPageButton.implicitWidth, Theme.viewNavigationButtonWidth)
        compare(previousPageButton.implicitHeight, Theme.buttonMinHeight)
    }

    function test_CTRL_FSB_003_disclosureTextFollowsExpandedState() {
        const button = createTemporaryObject(disclosureComponent, testCase)

        compare(button.text, "\u25B6")
        button.expanded = true
        compare(button.text, "\u25BC")
    }
}
