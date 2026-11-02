#include "core/options.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

TEST(OptionsTest, DefaultsToTopEdge) {
  const ParseResult result = ParseOptions({});
  EXPECT_EQ(result.options.edge, Edge::Top);
  EXPECT_TRUE(result.errors.empty());
}

TEST(OptionsTest, ParsesBottomEdge) {
  const ParseResult result = ParseOptions({L"--edge=bottom"});
  EXPECT_EQ(result.options.edge, Edge::Bottom);
  EXPECT_TRUE(result.errors.empty());
}

TEST(OptionsTest, DefaultsToSystemTheme) {
  EXPECT_EQ(ParseOptions({}).options.theme, ThemeMode::System);
}

TEST(OptionsTest, ParsesThemes) {
  EXPECT_EQ(ParseOptions({L"--theme=light"}).options.theme, ThemeMode::Light);
  EXPECT_EQ(ParseOptions({L"--theme=dark"}).options.theme, ThemeMode::Dark);
  EXPECT_EQ(ParseOptions({L"--theme=dark", L"--theme=system"}).options.theme, ThemeMode::System);
}

TEST(OptionsTest, ReportsUnknownArguments) {
  const ParseResult result = ParseOptions({L"--edge=left", L"--verbose"});
  ASSERT_EQ(result.errors.size(), 2u);
  EXPECT_EQ(result.errors[0], L"Unknown argument: --edge=left");
}

}  // namespace
}  // namespace throwback
