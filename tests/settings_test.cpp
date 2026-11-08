#include "core/settings.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

TEST(SettingsTest, RoundTripsThroughValues) {
  Settings settings;
  settings.edge = Edge::Bottom;
  settings.theme = ThemeMode::Dark;
  settings.language = LanguageMode::Turkish;
  settings.hideSystemTaskbar = true;
  settings.windowsKeyOpensMenu = false;
  settings.startWithWindows = false;
  settings.welcomeShown = true;
  settings.showSearchButton = false;
  settings.showTaskViewButton = false;
  settings.taskbarLocked = false;
  settings.startStyle = StartStyle::WinXp;
  settings.pinned = {L"C:\\a.lnk", L"shell:AppsFolder\\App!Id"};
  settings.trayShown = {L"c:\\apps\\a.exe|1"};
  settings.trayHidden = {L"g1.2.3.", L"c:\\apps\\b.exe|0"};
  settings.menuHorizontal = MenuHorizontal::Right;
  settings.menuVertical = MenuVertical::Top;
  settings.orb = OrbChoice::Clover;
  settings.menuRows = 14;
  settings.largeIcons = false;
  settings.placesShown = {L"videos"};
  settings.placesHidden = {L"music", L"run"};
  EXPECT_EQ(FromValues(ToValues(settings)), settings);
}

TEST(SettingsTest, MenuRowsStayInRange) {
  EXPECT_EQ(FromValues({{L"MenuRows", L"3"}}).menuRows, kDefaultMenuRows);
  EXPECT_EQ(FromValues({{L"MenuRows", L"21"}}).menuRows, kDefaultMenuRows);
  EXPECT_EQ(FromValues({{L"MenuRows", L"x"}}).menuRows, kDefaultMenuRows);
  EXPECT_EQ(FromValues({{L"MenuRows", L"4"}}).menuRows, 4);
  EXPECT_EQ(FromValues({{L"MenuRows", L"20"}}).menuRows, 20);
}

TEST(SettingsTest, PlacesKeepDefaultsUntilChanged) {
  Settings settings;
  EXPECT_TRUE(IsPlaceShown(settings, L"music", true));
  EXPECT_FALSE(IsPlaceShown(settings, L"videos", false));
  SetPlaceShown(settings, L"music", false);
  SetPlaceShown(settings, L"videos", true);
  EXPECT_FALSE(IsPlaceShown(settings, L"music", true));
  EXPECT_TRUE(IsPlaceShown(settings, L"videos", false));
}

TEST(SettingsTest, TrayIconsFollowWindowsUntilMoved) {
  Settings settings;
  EXPECT_TRUE(IsTrayIconShown(settings, L"a|1", true));
  EXPECT_FALSE(IsTrayIconShown(settings, L"a|1", false));
  SetTrayIconShown(settings, L"a|1", false);
  EXPECT_FALSE(IsTrayIconShown(settings, L"a|1", true));
  SetTrayIconShown(settings, L"a|1", true);
  EXPECT_TRUE(IsTrayIconShown(settings, L"a|1", false));
  // Moving an icon keeps one entry for it.
  EXPECT_EQ(settings.trayShown, (std::vector<std::wstring>{L"a|1"}));
  EXPECT_TRUE(settings.trayHidden.empty());
}

TEST(SettingsTest, MissingOrBadValuesKeepDefaults) {
  EXPECT_EQ(FromValues({}), Settings{});
  const Settings settings =
      FromValues({{L"Edge", L"middle"}, {L"HideSystemTaskbar", L"yes"}, {L"Theme", L"dark"}});
  EXPECT_EQ(settings.edge, Edge::Top);
  EXPECT_FALSE(settings.hideSystemTaskbar);
  EXPECT_EQ(settings.theme, ThemeMode::Dark);
}

TEST(SettingsTest, PinnedListSkipsBlankLinesAndDuplicates) {
  const Settings settings = FromValues({{L"Pinned", L"a\n\nb\na\n"}});
  EXPECT_EQ(settings.pinned, (std::vector<std::wstring>{L"a", L"b"}));
}

TEST(SettingsTest, OverridesWinOnlyWhenGiven) {
  Settings saved;
  saved.edge = Edge::Bottom;
  saved.theme = ThemeMode::Light;
  Options options;
  options.theme = ThemeMode::Dark;
  const Settings result = WithOverrides(saved, options);
  EXPECT_EQ(result.edge, Edge::Bottom);
  EXPECT_EQ(result.theme, ThemeMode::Dark);
}

TEST(SettingsTest, PinAndUnpin) {
  Settings settings;
  Pin(settings, L"a");
  Pin(settings, L"b");
  Pin(settings, L"a");
  Pin(settings, L"");
  EXPECT_EQ(settings.pinned, (std::vector<std::wstring>{L"a", L"b"}));
  EXPECT_TRUE(IsPinned(settings, L"b"));
  Unpin(settings, L"a");
  Unpin(settings, L"missing");
  EXPECT_EQ(settings.pinned, (std::vector<std::wstring>{L"b"}));
}

}  // namespace
}  // namespace throwback
