#include "ui/theme.h"

#include <gtest/gtest.h>

namespace throwback::ui {
namespace {

TEST(AeroThemeTest, GlassIsTranslucent) {
  // Opaque glass would hide the blurred desktop behind the bar.
  EXPECT_LT(aero::kGlassVeil.a, 1.0f);
  EXPECT_LT(aero::kGlassTint.a, 1.0f);
  EXPECT_GT(aero::kGlassVeil.a, 0.0f);
}

TEST(AeroThemeTest, PaneIsOpaqueWhiteWithDarkText) {
  EXPECT_EQ(aero::kPane, FromRgb(0xFFFFFF));
  EXPECT_EQ(aero::kText, FromRgb(0x000000));
  EXPECT_EQ(aero::kGlassText, FromRgb(0xFFFFFF));
}

TEST(AeroThemeTest, FromRgbSplitsChannels) {
  const Color color = FromRgb(0x336699, 0.5f);
  EXPECT_FLOAT_EQ(color.r, 0x33 / 255.0f);
  EXPECT_FLOAT_EQ(color.g, 0x66 / 255.0f);
  EXPECT_FLOAT_EQ(color.b, 0x99 / 255.0f);
  EXPECT_FLOAT_EQ(color.a, 0.5f);
}

TEST(TypeTest, NinePointSegoeEverywhere) {
  EXPECT_EQ(FontSize(TextStyle::Body), 12.0f);
  EXPECT_EQ(FontSize(TextStyle::Hint), 12.0f);
  EXPECT_TRUE(IsItalic(TextStyle::Hint));
  EXPECT_FALSE(IsItalic(TextStyle::Body));
}

}  // namespace
}  // namespace throwback::ui
