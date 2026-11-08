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

TEST(LayoutTest, ShowDesktopSitsAtTheFarEndAndPushesTheClock) {
  BarMetrics metrics = TestMetrics();
  metrics.showDesktopWidth = 12;
  const BarLayout layout = ComputeLayout(metrics, 2);
  EXPECT_EQ(layout.showDesktop, (Rect{988, 0, 1000, 40}));
  EXPECT_EQ(layout.clock, (Rect{888, 0, 988, 40}));
  EXPECT_LE(layout.taskButtons.back().right, layout.clock.left - metrics.spacing);
}

TEST(LayoutTest, TraySitsLeftOfTheClockAndPushesButtons) {
  BarMetrics metrics = TestMetrics();
  metrics.trayWidth = 96;
  const BarLayout layout = ComputeLayout(metrics, 50);
  EXPECT_EQ(layout.tray, (Rect{804, 0, 900, 40}));
  EXPECT_LE(layout.taskButtons.back().right, layout.tray.left - metrics.spacing);
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

TEST(VerticalLayoutTest, StacksSquareButtonsBetweenStartAndClock) {
  BarMetrics m;
  m.barWidth = 48;
  m.barHeight = 400;
  m.showDesktopWidth = 10;
  m.spacing = 2;
  m.padding = 2;
  const BarLayout layout = ComputeVerticalLayout(m, 40, 30, 20, 3);
  EXPECT_EQ(layout.startButton, (Rect{0, 0, 48, 48}));
  EXPECT_EQ(layout.showDesktop, (Rect{0, 390, 48, 400}));
  EXPECT_EQ(layout.notifications, (Rect{0, 370, 48, 390}));
  EXPECT_EQ(layout.clock, (Rect{0, 330, 48, 370}));
  EXPECT_EQ(layout.tray, (Rect{0, 300, 48, 330}));
  ASSERT_EQ(layout.taskButtons.size(), 3u);
  EXPECT_EQ(layout.taskButtons[0], (Rect{2, 50, 46, 94}));
  EXPECT_EQ(layout.taskButtons[1].top, 96);
}

TEST(VerticalLayoutTest, DropsButtonsThatDoNotFit) {
  BarMetrics m;
  m.barWidth = 48;
  m.barHeight = 240;
  const BarLayout layout = ComputeVerticalLayout(m, 40, 0, 0, 10);
  // 48 px for the start button and 40 for the clock leave three 48 px rows.
  EXPECT_EQ(layout.taskButtons.size(), 3u);
}

TEST(PlacePopupTest, OpensAwayFromTheBar) {
  const Rect screen{0, 0, 1000, 800};
  const Rect button{100, 760, 160, 800};
  EXPECT_EQ(PlacePopup(button, 200, 100, Edge::Bottom, 4, screen), (Point{30, 656}));
  EXPECT_EQ(PlacePopup({100, 0, 160, 40}, 200, 100, Edge::Top, 4, screen), (Point{30, 44}));
  EXPECT_EQ(PlacePopup({0, 300, 62, 340}, 200, 100, Edge::Left, 4, screen), (Point{66, 270}));
  EXPECT_EQ(PlacePopup({938, 300, 1000, 340}, 200, 100, Edge::Right, 4, screen), (Point{734, 270}));
}

TEST(PlacePopupTest, StaysOnScreen) {
  const Rect screen{0, 0, 1000, 800};
  EXPECT_EQ(PlacePopup({0, 760, 40, 800}, 200, 100, Edge::Bottom, 0, screen).x, 0);
  EXPECT_EQ(PlacePopup({0, 0, 62, 40}, 200, 100, Edge::Left, 0, screen).y, 0);
}

}  // namespace
}  // namespace throwback
