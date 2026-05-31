/**
 * @file ui/tests/unit/TestSettingsState.cpp
 * @brief Tests for the UI SettingsState boundary.
 */

#include <gtest/gtest.h>

#include <QApplication>
#include <QCoreApplication>
#include <QStringList>
#include <QVariantMap>

#include "ui/shell/AppActions.h"
#include "ui/state/navigation/NavigationState.h"
#include "ui/state/settings/SettingsState.h"
#include "ui/viewmodels/system/SettingsViewModel.h"

namespace ui {

namespace {

QVariantMap rowAt(const QVariantList& rows, int index)
{
    return rows.at(index).toMap();
}

void ensureQtApplication()
{
    if (QCoreApplication::instance()) {
        return;
    }

    static int argc = 1;
    static char appName[] = "ui_unit_tests";
    static char* argv[] = {appName, nullptr};
    static QApplication app(argc, argv);
}

} // namespace

TEST(SettingsStateTest, SET_ST_001_CategoryNavigationWrapsThroughNavigationState)
{
    NavigationState navigation;
    SettingsState state;
    state.setNavigation(&navigation);

    state.selectCategory(3);
    state.navigateCategory(1);
    EXPECT_EQ(state.currentCategory(), 0);

    state.navigateCategory(-1);
    EXPECT_EQ(state.currentCategory(), 3);
}

TEST(SettingsStateTest, SET_ST_002_CategoryRowsMarkSelectedCategory)
{
    NavigationState navigation;
    SettingsState state;
    state.setNavigation(&navigation);

    state.selectCategory(2);
    const QVariantList rows = state.categoryRows();

    ASSERT_EQ(rows.size(), 4);
    EXPECT_EQ(rowAt(rows, 2).value(QStringLiteral("category")).toInt(), 2);
    EXPECT_TRUE(rowAt(rows, 2).value(QStringLiteral("selected")).toBool());
    EXPECT_FALSE(rowAt(rows, 0).value(QStringLiteral("selected")).toBool());
}

TEST(SettingsStateTest, SET_ST_003_FileSelectionsUpdateSettingsViewModel)
{
    ensureQtApplication();

    SettingsViewModel settings;
    Actions actions;
    SettingsState state;
    state.setSettingsViewModel(&settings);
    state.setActions(&actions);

    emit actions.importFileSelected(QStringLiteral("test:///single.pdf"));
    EXPECT_EQ(settings.importDefaultPath(), QStringLiteral("test:///single.pdf"));

    emit actions.importFilesSelected(
        QStringList{QStringLiteral("test:///first.pdf"), QStringLiteral("test:///second.pdf")});
    EXPECT_EQ(settings.importDefaultPath(), QStringLiteral("test:///first.pdf"));

    emit actions.exportDirectorySelected(QStringLiteral("test:///exports"));
    EXPECT_EQ(settings.exportDefaultDirectory(), QStringLiteral("test:///exports"));
}

TEST(SettingsStateTest, SET_ST_004_SettingsPropertiesDelegateToViewModel)
{
    SettingsViewModel settings;
    SettingsState state;
    state.setSettingsViewModel(&settings);

    state.setExportArchiveFormat(1);
    state.setExportIncludeFormulas(false);
    state.setToolbarShowBooking(false);
    state.setImportMatcher(QStringLiteral("matcher-default"));
    state.setThemeMode(QStringLiteral("dark"));

    EXPECT_EQ(settings.exportArchiveFormat(), 1);
    EXPECT_FALSE(settings.exportIncludeFormulas());
    EXPECT_FALSE(settings.toolbarShowBooking());
    EXPECT_EQ(settings.importMatcher(), QStringLiteral("matcher-default"));
    EXPECT_EQ(settings.themeMode(), QStringLiteral("dark"));
    EXPECT_EQ(state.themeModeIndex(), 1);

    state.selectThemeModeAt(0);
    EXPECT_EQ(settings.themeMode(), QStringLiteral("light"));
}

} // namespace ui
