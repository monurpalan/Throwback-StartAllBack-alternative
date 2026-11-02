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
  settings.pinned = {L"C:\\a.lnk", L"shell:AppsFolder\\App!Id"};
  EXPECT_EQ(FromValues(ToValues(settings)), settings);
}

TEST(SettingsTest, MissingOrBadValuesKeepDefaults) {
  EXPECT_EQ(FromValues({}), Settings{});
  const Settings settings =
      FromValues({{L"Edge", L"left"}, {L"HideSystemTaskbar", L"yes"}, {L"Theme", L"dark"}});
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
