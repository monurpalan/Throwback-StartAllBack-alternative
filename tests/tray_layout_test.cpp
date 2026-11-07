#include "core/tray_layout.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

TEST(TrayLayoutTest, ArrowOnlyWhenIconsAreHidden) {
  EXPECT_EQ(TrayAreaWidth(0, false, 96), 0);
  EXPECT_EQ(TrayAreaWidth(2, false, 96), 56);
  EXPECT_EQ(TrayAreaWidth(2, true, 96), 76);
  EXPECT_EQ(TrayAreaWidth(0, true, 96), 28);

  const TrayAreaLayout plain = ComputeTrayArea({100, 0, 156, 40}, 2, false, 96);
  EXPECT_EQ(plain.arrow, Rect{});
  EXPECT_EQ(plain.icons.at(0), (Rect{104, 8, 128, 32}));

  const TrayAreaLayout hidden = ComputeTrayArea({100, 0, 176, 40}, 2, true, 96);
  EXPECT_EQ(hidden.arrow, (Rect{104, 0, 124, 40}));
  EXPECT_EQ(hidden.icons.at(0), (Rect{124, 8, 148, 32}));
  EXPECT_EQ(hidden.icons.at(1), (Rect{148, 8, 172, 32}));
}

TEST(TrayLayoutTest, FlyoutIsANearSquareGrid) {
  const TrayFlyoutLayout five = ComputeTrayFlyout(5, 0, 96);
  ASSERT_EQ(five.icons.size(), 5u);
  // Three columns, two rows.
  EXPECT_EQ(five.icons[2].top, five.icons[0].top);
  EXPECT_EQ(five.icons[3].top, five.icons[0].bottom);
  EXPECT_EQ(five.icons[3].left, five.icons[0].left);
  for (const Rect& icon : five.icons) {
    EXPECT_GE(icon.left, 0);
    EXPECT_LE(icon.right, five.frame.right);
    EXPECT_LE(icon.bottom, five.link.top);
  }
  EXPECT_EQ(five.link.bottom, five.frame.bottom);
}

TEST(TrayLayoutTest, FlyoutIsWideEnoughForTheLink) {
  const TrayFlyoutLayout one = ComputeTrayFlyout(1, 100, 96);
  EXPECT_EQ(one.frame.Width(), 124);
  // The icon stays centered.
  EXPECT_EQ(one.icons[0].left + one.icons[0].right, one.frame.Width());
}

}  // namespace
}  // namespace throwback
