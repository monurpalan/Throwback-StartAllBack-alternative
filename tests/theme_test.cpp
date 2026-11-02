#include "ui/theme.h"

#include <gtest/gtest.h>

#include <initializer_list>

namespace throwback::ui {
namespace {

bool IsPaletteColor(const Color& color) {
  for (const Color& base : {palette::kInk, palette::kPaper, palette::kAccent}) {
    if (color.SameRgb(base)) return true;
  }
  return false;
}

class ThemeTest : public ::testing::TestWithParam<bool> {};

TEST_P(ThemeTest, EveryRoleIsAPaletteColorWithAlpha) {
  const Theme theme = MakeTheme(GetParam());
  for (const Color& color : {theme.background, theme.foreground, theme.hover, theme.selected,
                             theme.selectedHover, theme.attention, theme.divider, theme.accent}) {
    EXPECT_TRUE(IsPaletteColor(color));
  }
}

TEST_P(ThemeTest, ForegroundContrastsWithBackground) {
  const Theme theme = MakeTheme(GetParam());
  EXPECT_FALSE(theme.foreground.SameRgb(theme.background));
  EXPECT_TRUE(theme.hover.SameRgb(theme.foreground));
  EXPECT_LT(theme.hover.a, theme.selected.a);
  EXPECT_LT(theme.selected.a, theme.selectedHover.a);
}

INSTANTIATE_TEST_SUITE_P(LightAndDark, ThemeTest, ::testing::Bool());

TEST(ThemeModeTest, LightAndDarkSwapInkAndPaper) {
  EXPECT_EQ(MakeTheme(true).background, palette::kPaper);
  EXPECT_EQ(MakeTheme(false).background, palette::kInk);
  EXPECT_EQ(MakeTheme(true).accent, MakeTheme(false).accent);
}

TEST(TypeScaleTest, OnlyFourSizesExist) {
  for (TextStyle style : {TextStyle::Title, TextStyle::Subtitle, TextStyle::Body, TextStyle::Small,
                          TextStyle::Label, TextStyle::Number, TextStyle::SmallNumber}) {
    const float size = FontSize(style);
    EXPECT_TRUE(size == 18.0f || size == 15.0f || size == 13.0f || size == 11.0f);
  }
}

TEST(TypeScaleTest, NumbersAndLabelsAreMono) {
  EXPECT_TRUE(IsMono(TextStyle::Number));
  EXPECT_TRUE(IsMono(TextStyle::SmallNumber));
  EXPECT_TRUE(IsMono(TextStyle::Label));
  EXPECT_FALSE(IsMono(TextStyle::Body));
  EXPECT_TRUE(IsUppercase(TextStyle::Label));
  EXPECT_EQ(FontSize(TextStyle::Label), kSmallSize);
}

TEST(SpacingTest, ScaleUsesMultiplesOfFour) {
  for (int space : {kSpace4, kSpace8, kSpace12, kSpace16, kSpace24, kSpace32, kSpace40, kSpace56}) {
    EXPECT_EQ(space % 4, 0);
  }
}

}  // namespace
}  // namespace throwback::ui
