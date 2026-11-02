#include "core/options.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

TEST(OptionsTest, Defaults) {
  const ParseResult result = ParseOptions({});
  EXPECT_EQ(result.options.edge, Edge::Top);
  EXPECT_EQ(result.options.theme, ThemeMode::System);
  EXPECT_EQ(result.options.language, LanguageMode::System);
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

TEST(OptionsTest, ReportsUnknownArguments) {
  const ParseResult result = ParseOptions({L"--edge=left", L"--verbose", L"--lang=de"});
  ASSERT_EQ(result.unknownArguments.size(), 3u);
  EXPECT_EQ(result.unknownArguments[0], L"--edge=left");
  EXPECT_EQ(result.unknownArguments[2], L"--lang=de");
}

}  // namespace
}  // namespace throwback
