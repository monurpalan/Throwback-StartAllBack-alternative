#include "core/options.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

TEST(OptionsTest, DefaultsOverrideNothing) {
  const ParseResult result = ParseOptions({});
  EXPECT_FALSE(result.options.edge);
  EXPECT_FALSE(result.options.theme);
  EXPECT_FALSE(result.options.language);
  EXPECT_EQ(result.options.profile, L"default");
  EXPECT_TRUE(result.unknownArguments.empty());
}

TEST(OptionsTest, ParsesBottomEdge) {
  const ParseResult result = ParseOptions({L"--edge=bottom"});
  EXPECT_EQ(result.options.edge, Edge::Bottom);
  EXPECT_TRUE(result.unknownArguments.empty());
}

TEST(OptionsTest, ParsesThemes) {
  EXPECT_EQ(ParseOptions({L"--theme=light"}).options.theme, ThemeMode::Light);
  EXPECT_EQ(ParseOptions({L"--theme=dark"}).options.theme, ThemeMode::Dark);
  EXPECT_EQ(ParseOptions({L"--theme=dark", L"--theme=system"}).options.theme, ThemeMode::System);
}

TEST(OptionsTest, ParsesLanguages) {
  EXPECT_EQ(ParseOptions({L"--lang=tr"}).options.language, LanguageMode::Turkish);
  EXPECT_EQ(ParseOptions({L"--lang=en"}).options.language, LanguageMode::English);
  EXPECT_EQ(ParseOptions({L"--lang=tr", L"--lang=system"}).options.language, LanguageMode::System);
}

TEST(OptionsTest, ParsesProfile) {
  EXPECT_EQ(ParseOptions({L"--profile=test"}).options.profile, L"test");
  // Profile names become registry key names, so separators are refused.
  EXPECT_EQ(ParseOptions({L"--profile=..\\other"}).unknownArguments.size(), 1u);
  EXPECT_EQ(ParseOptions({L"--profile="}).unknownArguments.size(), 1u);
}

TEST(OptionsTest, ValidatesProfileNames) {
  EXPECT_TRUE(IsValidProfileName(L"test_2-b"));
  EXPECT_FALSE(IsValidProfileName(L"a b"));
  EXPECT_FALSE(IsValidProfileName(L"a\\b"));
  EXPECT_FALSE(IsValidProfileName(std::wstring(33, L'a')));
}

TEST(OptionsTest, ParsesStartStyles) {
  EXPECT_EQ(ParseOptions({L"--start-style=98"}).options.startStyle, StartStyle::Win98);
  EXPECT_EQ(ParseOptions({L"--start-style=8.1"}).options.startStyle, StartStyle::Win81);
  EXPECT_EQ(ParseOptions({L"--start-style=11"}).options.startStyle, StartStyle::Win11);
  EXPECT_EQ(ParseOptions({L"--start-style=95"}).unknownArguments.size(), 1u);
  for (int i = 0; i <= static_cast<int>(StartStyle::Win11); ++i) {
    const auto style = static_cast<StartStyle>(i);
    EXPECT_EQ(ParseStartStyle(StartStyleName(style)), style);
  }
}

TEST(OptionsTest, ReportsUnknownArguments) {
  const ParseResult result = ParseOptions({L"--edge=middle", L"--verbose", L"--lang=de"});
  ASSERT_EQ(result.unknownArguments.size(), 3u);
  EXPECT_EQ(result.unknownArguments[0], L"--edge=middle");
  EXPECT_EQ(result.unknownArguments[2], L"--lang=de");
}

}  // namespace
}  // namespace throwback
