#include "core/layout.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

BarMetrics TestMetrics() {
  return {
      .barWidth = 1000,
      .barHeight = 40,
      .startButtonWidth = 50,
      .clockWidth = 100,
      .minButtonWidth = 60,
      .maxButtonWidth = 200,
      .spacing = 10,
      .padding = 2,
  };
}

TEST(LayoutTest, PlacesStartButtonLeftAndClockRight) {
  const BarLayout layout = ComputeLayout(TestMetrics(), 0);
  EXPECT_EQ(layout.startButton, (Rect{0, 0, 50, 40}));
  EXPECT_EQ(layout.clock, (Rect{900, 0, 1000, 40}));
  EXPECT_TRUE(layout.taskButtons.empty());
}

TEST(LayoutTest, FewButtonsUseMaxWidth) {
  const BarLayout layout = ComputeLayout(TestMetrics(), 2);
  ASSERT_EQ(layout.taskButtons.size(), 2u);
  EXPECT_EQ(layout.taskButtons[0], (Rect{60, 2, 260, 38}));
  EXPECT_EQ(layout.taskButtons[1], (Rect{270, 2, 470, 38}));
}

TEST(LayoutTest, ManyButtonsShrinkEvenly) {
  // Area between start button and clock is 60..890 = 830px.
  const BarLayout layout = ComputeLayout(TestMetrics(), 8);
  ASSERT_EQ(layout.taskButtons.size(), 8u);
  const int width = layout.taskButtons[0].Width();
  EXPECT_EQ(width, (830 - 7 * 10) / 8);
  for (const Rect& button : layout.taskButtons) EXPECT_EQ(button.Width(), width);
  EXPECT_LE(layout.taskButtons.back().right, 890);
}

TEST(LayoutTest, ButtonsThatDoNotFitAreDropped) {
  const BarLayout layout = ComputeLayout(TestMetrics(), 50);
  // 830px fits exactly 12 buttons of 60px with 10px spacing (12 * 60 + 11 * 10).
  ASSERT_EQ(layout.taskButtons.size(), 12u);
  EXPECT_EQ(layout.taskButtons[0].Width(), 60);
  EXPECT_LE(layout.taskButtons.back().right, 890);
}

TEST(LayoutTest, NoRoomForButtons) {
  BarMetrics metrics = TestMetrics();
  metrics.barWidth = 150;
  EXPECT_TRUE(ComputeLayout(metrics, 3).taskButtons.empty());
}

TEST(LayoutTest, CentersSquareInRect) {
  EXPECT_EQ(CenteredSquare({0, 0, 48, 40}, 16), (Rect{16, 12, 32, 28}));
  EXPECT_EQ(CenteredSquare({100, 10, 120, 30}, 20), (Rect{100, 10, 120, 30}));
}

TEST(LayoutTest, ScalesForDpi) {
  EXPECT_EQ(ScaleForDpi(40, 96), 40);
  EXPECT_EQ(ScaleForDpi(40, 144), 60);
  EXPECT_EQ(ScaleForDpi(40, 120), 50);
  EXPECT_EQ(ScaleMetrics(TestMetrics(), 192).clockWidth, 200);
}

}  // namespace
}  // namespace throwback
