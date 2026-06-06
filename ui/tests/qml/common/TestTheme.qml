/**
 * @file ui/tests/qml/common/TestTheme.qml
 * @brief Provides the complete FossRedder theme contract for isolated QML tests.
 */

import QtQuick 2.15

QtObject {
    readonly property string lightMode: "light"
    readonly property string darkMode: "dark"
    property string mode: lightMode
    readonly property bool dark: mode === darkMode

    property color primary: "#3E6F8F"
    property color accent: "#5AA8A5"
    property color background: "#EEF3F5"
    property color surface: "#FBFCFD"
    property color surfaceAlt: "#F4F8FA"
    property color border: "#D3DEE4"
    property color textPrimary: "#22313A"
    property color textMuted: "#657781"

    property color danger: "#B45A54"
    property color success: "#2F8061"
    property color warning: "#A36F22"
    property color successStrong: "#3B8E6C"
    property color borderSoft: "#E0E8EC"
    property color borderLight: "#EAF0F3"
    property color borderMedium: "#CAD8DF"
    property color borderStrong: "#AFC2CD"
    property color statusBarBackground: "#E8F0F4"
    property color statusBarBorder: "#D3E1E8"
    property color placeholderText: "#87949B"
    property color selectionHighlight: "#D7E8F3"
    property color selectionBorder: "#6EA1BF"
    property color sidebarHoverFill: "#F6FAFB"
    property color sidebarHoverBorder: "#B8CCD6"
    property color toolbarBackgroundTop: "#F3F8FA"
    property color toolbarBackgroundBottom: "#E4EEF3"
    property color toolbarItemText: "#536872"
    property color toolbarItemActiveText: "#1F3F52"
    property color toolbarItemHoverFill: "#F7FBFC"
    property color toolbarItemSelectedFill: "#D7EAF2"
    property color toolbarItemSelectedBorder: "#8EB5C8"
    property color toolbarGroupActiveText: "#3C728C"
    property color appMenuBackgroundTop: "#F7FAFB"
    property color appMenuBackgroundBottom: "#EAF2F5"
    property color appMenuText: "#2D3D46"
    property real panelShadowOpacity: 0.025
    property color statusSuccessFill: "#EAF4EE"
    property color statusSuccessText: "#2F8061"
    property color statusSuccessBorder: "#BCD9C8"
    property color statusWarningFill: "#F7F0E2"
    property color statusWarningText: "#8C6425"
    property color statusWarningBorder: "#DEC994"
    property color statusDangerFill: "#F6ECEA"
    property color statusDangerText: "#9A4A45"
    property color statusDangerBorder: "#D8BDB8"
    property color statusInfoFill: "#E8F1F6"
    property color statusInfoText: "#416F89"
    property color statusInfoBorder: "#BCD0DA"
    property color statusNeutralFill: "#F4F8FA"
    property color statusNeutralText: "#657781"
    property color statusNeutralBorder: "#D3DEE4"
    property color toolbarBorder: "#D2E1E8"
    property color divider: "#B9CDD8"
    property color shadow: "#000000"
    property color subtlePrimaryFill: "#DDECF3"
    property color buttonFill: "#F8FBFC"
    property color buttonText: "#243645"
    property color buttonMutedText: "#586B75"
    property color buttonConfirmFill: "#D9E9F0"
    property color buttonConfirmText: "#213F52"
    property color buttonConfirmBorder: "#AEC9D6"
    property color controlFill: "#FBFCFD"
    property color controlHoverFill: "#F6FAFB"
    property color controlFocusBorder: "#6EA1BF"
    property color controlHoverBorder: "#B8CCD6"
    property real controlShadowOpacity: 0.055
    property real popupShadowOpacity: 0.11
    property color checkboxFill: "#F8FBFC"
    property color checkboxHoverFill: "#EEF6F8"
    property color checkboxCheckedFill: "#3E6F8F"
    property color checkboxBorder: "#AFC2CD"
    property color checkboxCheck: "#FFFFFF"
    property color scrollBarThumb: "#C4D6DE"
    property color scrollBarThumbHover: "#AEC9D6"
    property color menuFill: "#FBFCFD"
    property color menuHoverFill: "#DDECF3"
    property color appMenuPopupFill: "#FBFCFD"
    property color appMenuPopupBorder: "#C9D9E1"
    property color appMenuPopupText: "#263843"
    property color appMenuPopupMutedText: "#697B85"
    property color appMenuPopupHoverFill: "#E4F0F5"
    property color appMenuSeparator: "#D8E4EA"

    property int margins: 2
    property int spacing: 12
    property int spacingMedium: 8
    property int spacingSmall: 6
    property int spacingLarge: 20
    property int shellMinimumHeight: 100
    property int shellSidebarMinimumWidth: 100
    property int shellSidebarPreferredWidth: 320
    property int shellContentMinimumWidth: 200
    property int toolbarHeight: 80
    property int toolbarIconButtonWidth: 72
    property int toolbarIconRowHeight: 62
    property int toolbarIconSize: 32
    property int toolbarGroupSpacing: 12
    property int toolbarSectionSpacing: 2
    property int toolbarItemFrameTopInset: 6
    property int toolbarContentBottomInset: 6
    property int statusBarHeight: 36
    property int formLabelWidth: 120
    property int formFieldWidth: 200
    property int chartPlotPreferredHeight: 320
    property int chartPlotMinimumHeight: 200
    property int viewNavigationButtonWidth: 42
    property int panelContentSafeWidthOffset: 24
    property int viewActionButtonWidth: 120
    property int viewAliasGroupSpacing: 6
    property int viewAliasPanelMinHeight: 160
    property int viewAliasPanelPreferredHeight: 180
    property int viewSidebarRowHeight: 44
    property int viewSidebarRowRadius: 6
    property int viewSidebarRowSpacing: 2
    property int viewSidebarOuterVerticalInset: pageContentMargin * 2
    property int viewSidebarEntryInset: toolbarGroupSpacing
    property int viewSidebarScrollBarGap: spacingSmall
    property int viewSidebarScrollBarOuterInset: toolbarGroupSpacing
    property int viewSidebarEntryInsetTotal: viewSidebarEntryInset + viewSidebarScrollBarGap + scrollBarGutterWidth + viewSidebarScrollBarOuterInset
    property int viewAliasChipHeight: 30
    property int viewAliasChipRadius: 6
    property int viewSelectionPanelMinHeight: 170
    property int viewSelectionPanelPreferredHeight: 220
    property int controlHeight: 40
    property int controlPaddingHorizontal: 8
    property int controlPaddingVertical: 6
    property int buttonMinHeight: 36
    property int checkboxSize: 18
    property int checkListPanelMinWidth: 220
    property int checkListPanelMaxVisibleRows: 5
    property int scrollBarGutterWidth: 14
    property int scrollBarThumbWidth: 6
    property real scrollBarThumbRightInset: margins
    property real scrollBarThumbEndInset: borderWidthThin
    property real scrollBarMinimumSize: 0.08
    property int menuItemHeight: 30
    property int menuItemMinWidth: 150
    property int appMenuPopupPadding: 6
    property int appMenuPopupRadius: 3
    property int appMenuPopupMinWidth: 176
    property int appMenuItemHeight: 28
    property int appMenuItemRadius: 3
    property int appMenuItemHorizontalPadding: 10
    property int appMenuItemCheckWidth: 18
    property int appMenuItemShortcutSpacing: 28
    property int appMenuFontSize: 9
    property int panelPadding: 12
    property int pageContentMargin: 8
    property int viewSectionIconSize: 28
    property int viewCompactActionButtonSize: 40
    property int viewCompactActionButtonSizeSmall: 32
    property int viewCompactActionButtonSizeTiny: 22
    property int dropdownIndicatorWidth: 36
    property int dropdownPopupMaxHeight: 280
    property int popupZ: 999
    property int animationDurationFast: 160
    property int radius: 3
    property int borderWidthThin: 1

    property var analysis: ({
            palette: ["#8dd3c7", "#ffffb3", "#bebada", "#fb8072", "#80b1d3", "#fdb462", "#b3de69", "#fccde5", "#d9d9d9", "#bc80bd", "#ccebc5", "#ffed6f"],
            layout: {
                defaultWidth: 800,
                defaultHeight: 600,
                splitControlsWidth: 220,
                splitAnimationDurationMs: 400,
                initialPaintIntervalMs: 300,
                initialPaintReadyWidth: 120,
                initialPaintMaxAttempts: 6,
                repaintDelayMs: 100,
                minDebugRepaintWidth: 50,
                minRenderWidth: 100,
                legendTopMargin: 8
            },
            render: {
                pieStartAngle: -Math.PI / 2,
                pieRadiusPadding: 10,
                compactBarLeftPadding: 20,
                compactBarVerticalSpacing: 18,
                compactBarTopOffset: 4,
                compactBarHeight: 12,
                compactBarWidthPadding: 40,
                compactBarMinWidth: 2,
                histogramGroupPadding: 8,
                histogramBottomPadding: 18,
                histogramTopPadding: 30,
                propertyBarInset: 2,
                propertyLabelMinWidth: 36
            },
            table: {
                rowHeight: 28,
                dateColumnWidth: 140,
                amountColumnWidth: 100,
                contractColumnWidth: 160,
                propertyColumnWidth: 160,
                totalColumnWidth: 100,
                detailListHeight: 300
            },
            calc: {
                defaultWidth: 400,
                taxFieldWidth: 80,
                transactionListHeight: 220,
                nameColumnWidth: 200
            },
            transactions: {
                applyColumnWidth: 44,
                statementColumnWidth: 130,
                transactionColumnWidth: 150,
                dateColumnWidth: 110,
                actorColumnWidth: 120,
                contractColumnWidth: 120,
                typeColumnWidth: 90,
                propertiesColumnWidth: 220,
                amountColumnWidth: 160,
                columnSpacingCount: 9,
                horizontalPaddingCount: 2,
                headerHeight: 32,
                rowHeight: 30,
                adjustmentPercentFieldWidth: 90,
                metricsStatementWidth: 160,
                metricsTransactionWidth: 170,
                metricsAmountWidth: 180
            }
        })

    property var annual: ({
            transactions: {
                tableMinWidth: 720,
                dateColumnWidth: 110,
                amountColumnWidth: 90,
                allocatableColumnWidth: 130,
                typeColumnWidth: 120,
                statusColumnWidth: 100
            }
        })

    property var exportView: ({
            panel: {
                addModeButtonWidth: 88,
                addButtonWidth: 72,
                panelMinHeight: 320,
                objectListMinHeight: 180,
                exportTypeColumnWidth: 110,
                kindColumnWidth: 88,
                analysisNameMinWidth: 160
            }
        })

    property string fontFamily: "Segoe UI Variable Text"

    property int fontSizeSmall: 10
    property int fontSize: 11
    property int buttonFontSize: 10
    property int toolbarLabelFontSize: 9
    property int fontSizeTitle: 16
}
