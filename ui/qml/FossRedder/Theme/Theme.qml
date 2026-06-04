/**
 * @file ui/qml/FossRedder/Theme/Theme.qml
 * @brief Provides the Theme component.
 */

pragma Singleton

import QtQuick 2.15

QtObject {
    readonly property string lightMode: "light"
    readonly property string darkMode: "dark"
    property string mode: lightMode
    readonly property bool dark: mode === darkMode

    readonly property LightPalette lightPalette: LightPalette {}
    readonly property DarkPalette darkPalette: DarkPalette {}

    property color primary: dark ? darkPalette.primary : lightPalette.primary
    property color accent: dark ? darkPalette.accent : lightPalette.accent
    property color background: dark ? darkPalette.background : lightPalette.background
    property color surface: dark ? darkPalette.surface : lightPalette.surface
    property color surfaceAlt: dark ? darkPalette.surfaceAlt : lightPalette.surfaceAlt
    property color border: dark ? darkPalette.border : lightPalette.border
    property color textPrimary: dark ? darkPalette.textPrimary : lightPalette.textPrimary
    property color textMuted: dark ? darkPalette.textMuted : lightPalette.textMuted

    property color danger: dark ? darkPalette.danger : lightPalette.danger
    property color success: dark ? darkPalette.success : lightPalette.success
    property color warning: dark ? darkPalette.warning : lightPalette.warning
    property color successStrong: dark ? darkPalette.successStrong : lightPalette.successStrong
    property color borderSoft: dark ? darkPalette.borderSoft : lightPalette.borderSoft
    property color borderLight: dark ? darkPalette.borderLight : lightPalette.borderLight
    property color borderMedium: dark ? darkPalette.borderMedium : lightPalette.borderMedium
    property color borderStrong: dark ? darkPalette.borderStrong : lightPalette.borderStrong
    property color statusBarBackground: dark ? darkPalette.statusBarBackground : lightPalette.statusBarBackground
    property color statusBarBorder: dark ? darkPalette.statusBarBorder : lightPalette.statusBarBorder
    property color placeholderText: dark ? darkPalette.placeholderText : lightPalette.placeholderText
    property color selectionHighlight: dark ? darkPalette.selectionHighlight : lightPalette.selectionHighlight
    property color selectionBorder: dark ? darkPalette.selectionBorder : lightPalette.selectionBorder
    property color sidebarHoverFill: dark ? darkPalette.sidebarHoverFill : lightPalette.sidebarHoverFill
    property color sidebarHoverBorder: dark ? darkPalette.sidebarHoverBorder : lightPalette.sidebarHoverBorder
    property color toolbarBackgroundTop: dark ? darkPalette.toolbarBackgroundTop : lightPalette.toolbarBackgroundTop
    property color toolbarBackgroundBottom: dark ? darkPalette.toolbarBackgroundBottom : lightPalette.toolbarBackgroundBottom
    property color toolbarItemText: dark ? darkPalette.toolbarItemText : lightPalette.toolbarItemText
    property color toolbarItemActiveText: dark ? darkPalette.toolbarItemActiveText : lightPalette.toolbarItemActiveText
    property color toolbarItemHoverFill: dark ? darkPalette.toolbarItemHoverFill : lightPalette.toolbarItemHoverFill
    property color toolbarItemSelectedFill: dark ? darkPalette.toolbarItemSelectedFill : lightPalette.toolbarItemSelectedFill
    property color toolbarItemSelectedBorder: dark ? darkPalette.toolbarItemSelectedBorder : lightPalette.toolbarItemSelectedBorder
    property color toolbarGroupActiveText: dark ? darkPalette.toolbarGroupActiveText : lightPalette.toolbarGroupActiveText
    property color appMenuBackgroundTop: dark ? darkPalette.appMenuBackgroundTop : lightPalette.appMenuBackgroundTop
    property color appMenuBackgroundBottom: dark ? darkPalette.appMenuBackgroundBottom : lightPalette.appMenuBackgroundBottom
    property color appMenuText: dark ? darkPalette.appMenuText : lightPalette.appMenuText
    property real panelShadowOpacity: dark ? darkPalette.panelShadowOpacity : lightPalette.panelShadowOpacity
    property color statusSuccessFill: dark ? darkPalette.statusSuccessFill : lightPalette.statusSuccessFill
    property color statusSuccessText: dark ? darkPalette.statusSuccessText : lightPalette.statusSuccessText
    property color statusSuccessBorder: dark ? darkPalette.statusSuccessBorder : lightPalette.statusSuccessBorder
    property color statusWarningFill: dark ? darkPalette.statusWarningFill : lightPalette.statusWarningFill
    property color statusWarningText: dark ? darkPalette.statusWarningText : lightPalette.statusWarningText
    property color statusWarningBorder: dark ? darkPalette.statusWarningBorder : lightPalette.statusWarningBorder
    property color statusDangerFill: dark ? darkPalette.statusDangerFill : lightPalette.statusDangerFill
    property color statusDangerText: dark ? darkPalette.statusDangerText : lightPalette.statusDangerText
    property color statusDangerBorder: dark ? darkPalette.statusDangerBorder : lightPalette.statusDangerBorder
    property color statusInfoFill: dark ? darkPalette.statusInfoFill : lightPalette.statusInfoFill
    property color statusInfoText: dark ? darkPalette.statusInfoText : lightPalette.statusInfoText
    property color statusInfoBorder: dark ? darkPalette.statusInfoBorder : lightPalette.statusInfoBorder
    property color statusNeutralFill: dark ? darkPalette.statusNeutralFill : lightPalette.statusNeutralFill
    property color statusNeutralText: dark ? darkPalette.statusNeutralText : lightPalette.statusNeutralText
    property color statusNeutralBorder: dark ? darkPalette.statusNeutralBorder : lightPalette.statusNeutralBorder
    property color toolbarBorder: dark ? darkPalette.toolbarBorder : lightPalette.toolbarBorder
    property color divider: dark ? darkPalette.divider : lightPalette.divider
    property color shadow: dark ? darkPalette.shadow : lightPalette.shadow
    property color subtlePrimaryFill: dark ? darkPalette.subtlePrimaryFill : lightPalette.subtlePrimaryFill
    property color onPrimary: dark ? darkPalette.primaryText : lightPalette.primaryText
    property color buttonFill: dark ? darkPalette.buttonFill : lightPalette.buttonFill
    property color buttonText: dark ? darkPalette.buttonText : lightPalette.buttonText
    property color buttonMutedText: dark ? darkPalette.buttonMutedText : lightPalette.buttonMutedText
    property color buttonConfirmFill: dark ? darkPalette.buttonConfirmFill : lightPalette.buttonConfirmFill
    property color buttonConfirmText: dark ? darkPalette.buttonConfirmText : lightPalette.buttonConfirmText
    property color buttonConfirmBorder: dark ? darkPalette.buttonConfirmBorder : lightPalette.buttonConfirmBorder
    property color controlFill: dark ? darkPalette.controlFill : lightPalette.controlFill
    property color controlHoverFill: dark ? darkPalette.controlHoverFill : lightPalette.controlHoverFill
    property color controlFocusBorder: dark ? darkPalette.controlFocusBorder : lightPalette.controlFocusBorder
    property color controlHoverBorder: dark ? darkPalette.controlHoverBorder : lightPalette.controlHoverBorder
    property real controlShadowOpacity: dark ? darkPalette.controlShadowOpacity : lightPalette.controlShadowOpacity
    property real popupShadowOpacity: dark ? darkPalette.popupShadowOpacity : lightPalette.popupShadowOpacity
    property color checkboxFill: dark ? darkPalette.checkboxFill : lightPalette.checkboxFill
    property color checkboxHoverFill: dark ? darkPalette.checkboxHoverFill : lightPalette.checkboxHoverFill
    property color checkboxCheckedFill: dark ? darkPalette.checkboxCheckedFill : lightPalette.checkboxCheckedFill
    property color checkboxBorder: dark ? darkPalette.checkboxBorder : lightPalette.checkboxBorder
    property color checkboxCheck: dark ? darkPalette.checkboxCheck : lightPalette.checkboxCheck
    property color scrollBarThumb: dark ? darkPalette.scrollBarThumb : lightPalette.scrollBarThumb
    property color scrollBarThumbHover: dark ? darkPalette.scrollBarThumbHover : lightPalette.scrollBarThumbHover
    property color menuFill: dark ? darkPalette.menuFill : lightPalette.menuFill
    property color menuHoverFill: dark ? darkPalette.menuHoverFill : lightPalette.menuHoverFill
    property color appMenuPopupFill: dark ? darkPalette.appMenuPopupFill : lightPalette.appMenuPopupFill
    property color appMenuPopupBorder: dark ? darkPalette.appMenuPopupBorder : lightPalette.appMenuPopupBorder
    property color appMenuPopupText: dark ? darkPalette.appMenuPopupText : lightPalette.appMenuPopupText
    property color appMenuPopupMutedText: dark ? darkPalette.appMenuPopupMutedText : lightPalette.appMenuPopupMutedText
    property color appMenuPopupHoverFill: dark ? darkPalette.appMenuPopupHoverFill : lightPalette.appMenuPopupHoverFill
    property color appMenuSeparator: dark ? darkPalette.appMenuSeparator : lightPalette.appMenuSeparator

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
