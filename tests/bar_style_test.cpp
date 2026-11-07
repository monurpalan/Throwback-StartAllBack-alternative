#include "core/bar_style.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

TEST(BarStyleTest, EachGenerationHasItsHeight) {
  EXPECT_EQ(BarStyleOf(StartStyle::Win98).height, 28);
  EXPECT_EQ(BarStyleOf(StartStyle::Win2000).height, 28);
  EXPECT_EQ(BarStyleOf(StartStyle::WinXp).height, 30);
  EXPECT_EQ(BarStyleOf(StartStyle::Vista).height, 30);
  EXPECT_EQ(BarStyleOf(StartStyle::Win7).height, 40);
  EXPECT_EQ(BarStyleOf(StartStyle::Win10).height, 40);
  EXPECT_EQ(BarStyleOf(StartStyle::Win11).height, 48);
}

TEST(BarStyleTest, ModernBarsShowIconsOnly) {
  EXPECT_TRUE(BarStyleOf(StartStyle::Win7).labels);
  EXPECT_FALSE(BarStyleOf(StartStyle::Win10).labels);
  const BarMetrics metrics = BarStyleOf(StartStyle::Win11).metrics;
  EXPECT_EQ(metrics.minButtonWidth, metrics.maxButtonWidth);
}

TEST(BarStyleTest, OlderClocksShowTheTimeOnly) {
  EXPECT_FALSE(BarStyleOf(StartStyle::WinMe).dateLine);
  EXPECT_FALSE(BarStyleOf(StartStyle::WinXp).dateLine);
  EXPECT_TRUE(BarStyleOf(StartStyle::Win81).dateLine);
  EXPECT_EQ(BarFamilyOf(StartStyle::WinMe), BarFamily::Classic98);
  EXPECT_EQ(BarFamilyOf(StartStyle::Win81), BarFamily::Win8);
}

}  // namespace
}  // namespace throwback
