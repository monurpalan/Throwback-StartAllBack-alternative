#include "core/start_menu_layout.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

bool Inside(const Rect& inner, const Rect& outer) {
  return inner.left >= outer.left && inner.top >= outer.top && inner.right <= outer.right &&
         inner.bottom <= outer.bottom;
}

TEST(StartMenuLayoutTest, ClassicSizeAt96Dpi) {
  const StartMenuLayout layout = ComputeStartMenuLayout(96);
  EXPECT_EQ(layout.frame, (Rect{0, 0, 411, 518}));
  EXPECT_EQ(layout.leftPane, (Rect{8, 8, 261, 464}));
  EXPECT_EQ(layout.list, (Rect{8, 8, 261, 408}));
  EXPECT_EQ(layout.allPrograms, (Rect{8, 424, 261, 464}));
  EXPECT_EQ(layout.search, (Rect{21, 475, 243, 499}));
  EXPECT_EQ(layout.picture, (Rect{312, 8, 360, 56}));
  EXPECT_EQ(layout.power, (Rect{269, 475, 345, 499}));
  EXPECT_EQ(layout.powerArrow, (Rect{345, 475, 369, 499}));
  EXPECT_EQ(layout.jumpList, (Rect{269, 8, 403, 464}));
  EXPECT_EQ(layout.programsArea, (Rect{0, 0, 261, 464}));
  EXPECT_EQ(layout.searchArea, (Rect{0, 464, 261, 518}));
  EXPECT_EQ(layout.placesArea, (Rect{261, 0, 411, 464}));
  EXPECT_EQ(layout.powerArea, (Rect{261, 464, 411, 518}));
}

TEST(StartMenuLayoutTest, BackgroundAreasCoverTheFrame) {
  for (unsigned dpi : {96u, 120u, 144u, 192u}) {
    const StartMenuLayout layout = ComputeStartMenuLayout(dpi);
    long long area = 0;
    for (const Rect& part :
         {layout.programsArea, layout.searchArea, layout.placesArea, layout.powerArea}) {
      EXPECT_TRUE(Inside(part, layout.frame)) << dpi;
      area += static_cast<long long>(part.Width()) * part.Height();
    }
    EXPECT_EQ(area, static_cast<long long>(layout.frame.Width()) * layout.frame.Height()) << dpi;
    EXPECT_TRUE(Inside(layout.search, layout.searchArea)) << dpi;
    EXPECT_TRUE(Inside(layout.powerArrow, layout.powerArea)) << dpi;
  }
}

TEST(StartMenuLayoutTest, ShutDownButtonFitsItsLabel) {
  // Short labels keep the narrowest button, long ones grow it up to the
  // column's width.
  EXPECT_EQ(ComputeStartMenuLayout(96, 40).power.Width(), 76);
  EXPECT_EQ(ComputeStartMenuLayout(96, 70).power.Width(), 94);
  const StartMenuLayout widest = ComputeStartMenuLayout(96, 1000);
  EXPECT_EQ(widest.powerArrow.right, widest.frame.right - 8);
}

TEST(StartMenuLayoutTest, PartsStayInsideTheirAreas) {
  for (unsigned dpi : {96u, 120u, 144u, 192u}) {
    const StartMenuLayout layout = ComputeStartMenuLayout(dpi);
    EXPECT_TRUE(Inside(layout.list, layout.leftPane)) << dpi;
    EXPECT_TRUE(Inside(layout.allPrograms, layout.leftPane)) << dpi;
    EXPECT_LE(layout.list.bottom, layout.allPrograms.top) << dpi;
    EXPECT_GE(layout.search.top, layout.leftPane.bottom) << dpi;
    EXPECT_TRUE(Inside(layout.search, layout.frame)) << dpi;
    EXPECT_GE(layout.places.left, layout.leftPane.right) << dpi;
    EXPECT_EQ(layout.power.right, layout.powerArrow.left) << dpi;
    EXPECT_TRUE(Inside(layout.powerArrow, layout.frame)) << dpi;
    EXPECT_TRUE(Inside(layout.jumpList, layout.frame)) << dpi;
    EXPECT_GE(layout.jumpList.left, layout.leftPane.right) << dpi;
  }
}

TEST(StartMenuLayoutTest, FollowsItsShape) {
  StartMenuShape shape;
  shape.rows = 6;
  shape.largeIcons = false;
  shape.programPaneWidth = 311;
  shape.placesWidth = 195;
  const StartMenuLayout layout = ComputeStartMenuLayout(96, 0, shape);
  EXPECT_EQ(layout.frame.Width(), 506);
  EXPECT_EQ(layout.rowHeight, 24);
  EXPECT_EQ(layout.listRows, 6);
  EXPECT_EQ(layout.leftPane.right, 311);
  EXPECT_GE(layout.list.Height(), 6 * 24);
  // Many places make the menu taller than its rows.
  shape.placeRows = 14;
  shape.placeSeparators = 3;
  const StartMenuLayout tall = ComputeStartMenuLayout(96, 0, shape);
  EXPECT_GT(tall.frame.Height(), layout.frame.Height());
  EXPECT_GE(tall.places.Height(), 14 * tall.placeHeight + 3 * 8);
}

TEST(StartMenuLayoutTest, ScalesWithDpi) {
  EXPECT_EQ(ComputeStartMenuLayout(192).frame, (Rect{0, 0, 822, 1036}));
  EXPECT_EQ(ComputeStartMenuLayout(192).rowHeight, 80);
}

TEST(StartMenuLayoutTest, XpFollowsItsTheme) {
  const StartMenuLayout layout = ComputeXpStartMenuLayout(96, 40, 100);
  EXPECT_EQ(layout.frame, (Rect{0, 0, 380, 440}));
  EXPECT_EQ(layout.userPane, (Rect{0, 0, 380, 64}));
  EXPECT_EQ(layout.programsArea, (Rect{0, 64, 190, 370}));
  EXPECT_EQ(layout.allPrograms, (Rect{0, 370, 190, 400}));
  EXPECT_EQ(layout.placesArea, (Rect{190, 64, 380, 400}));
  EXPECT_EQ(layout.powerArea, (Rect{0, 400, 380, 440}));
  EXPECT_LE(layout.list.bottom, layout.allPrograms.top);
  // Turn Off Computer ends near the right edge, Log Off sits left of it.
  EXPECT_EQ(layout.powerArrow.right, 372);
  EXPECT_EQ(layout.powerArrow.Width(), 136);
  EXPECT_EQ(layout.power.right, layout.powerArrow.left);
  EXPECT_EQ(layout.power.Width(), 76);
  EXPECT_EQ(layout.search, Rect{});
}

TEST(StartMenuLayoutTest, VistaPutsSearchUnderThePane) {
  const StartMenuLayout layout = ComputeVistaStartMenuLayout(96);
  EXPECT_EQ(layout.frame, (Rect{0, 0, 411, 486}));
  EXPECT_EQ(layout.allPrograms, (Rect{8, 416, 261, 444}));
  EXPECT_GE(layout.search.top, layout.allPrograms.bottom);
  EXPECT_LE(layout.search.bottom, layout.frame.bottom);
  EXPECT_EQ(layout.power.right + 4, layout.lock.left);
  EXPECT_EQ(layout.lock.right + 4, layout.powerArrow.left);
  EXPECT_LE(layout.places.bottom, layout.powerArrow.top);
}

// A 1920 x 1080 screen with a 40 px bar at the bottom (start button at the
// left end) and a 100 x 200 menu.
constexpr Rect kScreen{0, 0, 1920, 1080};
constexpr Rect kBottomButton{0, 1040, 56, 1080};
constexpr Rect kBottomWork{0, 0, 1920, 1040};
constexpr Rect kTopButton{0, 0, 56, 40};
constexpr Rect kTopWork{0, 40, 1920, 1080};

TEST(PlaceStartMenuTest, FollowsTheBarByDefault) {
  EXPECT_EQ(PlaceStartMenu(kBottomButton, false, 100, 200, kScreen, kBottomWork,
                           MenuHorizontal::Left, MenuVertical::Bar),
            (MenuPosition{0, 840}));
  EXPECT_EQ(PlaceStartMenu(kTopButton, true, 100, 200, kScreen, kTopWork, MenuHorizontal::Left,
                           MenuVertical::Bar),
            (MenuPosition{0, 40}));
}

TEST(PlaceStartMenuTest, HugsTheRightEdge) {
  EXPECT_EQ(PlaceStartMenu(kBottomButton, false, 100, 200, kScreen, kBottomWork,
                           MenuHorizontal::Right, MenuVertical::Bar),
            (MenuPosition{1820, 840}));
}

TEST(PlaceStartMenuTest, PinsToTheOtherEdge) {
  // A bottom bar with the menu at the top of the screen, and the other way
  // round.
  EXPECT_EQ(PlaceStartMenu(kBottomButton, false, 100, 200, kScreen, kBottomWork,
                           MenuHorizontal::Left, MenuVertical::Top),
            (MenuPosition{0, 0}));
  EXPECT_EQ(PlaceStartMenu(kTopButton, true, 100, 200, kScreen, kTopWork, MenuHorizontal::Right,
                           MenuVertical::Bottom),
            (MenuPosition{1820, 880}));
  // Pinning to the bar's own side changes nothing.
  EXPECT_EQ(PlaceStartMenu(kBottomButton, false, 100, 200, kScreen, kBottomWork,
                           MenuHorizontal::Left, MenuVertical::Bottom),
            (MenuPosition{0, 840}));
}

TEST(PlaceStartMenuTest, KeepsAGapAndStaysOnScreen) {
  EXPECT_EQ(PlaceStartMenu(kBottomButton, false, 100, 200, kScreen, kBottomWork,
                           MenuHorizontal::Left, MenuVertical::Bar, 12),
            (MenuPosition{0, 828}));
  EXPECT_EQ(PlaceStartMenu(kBottomButton, false, 100, 2000, kScreen, kBottomWork,
                           MenuHorizontal::Left, MenuVertical::Bar),
            (MenuPosition{0, 0}));
}

TEST(PlaceStartMenuTest, OpensBesideAnUprightBar) {
  // A 62 px bar on the left: the start button collapsed to its top edge.
  const Rect leftWork{62, 0, 1920, 1080};
  EXPECT_EQ(PlaceStartMenu({0, 0, 62, 0}, true, 100, 200, kScreen, leftWork, MenuHorizontal::Left,
                           MenuVertical::Bar),
            (MenuPosition{62, 0}));
  // On the right, a menu on the left side still stays off the bar.
  const Rect rightWork{0, 0, 1858, 1080};
  EXPECT_EQ(PlaceStartMenu({1858, 0, 1920, 0}, true, 100, 200, kScreen, rightWork,
                           MenuHorizontal::Left, MenuVertical::Bar),
            (MenuPosition{1758, 0}));
}

TEST(XpStartMenuLayoutTest, GrowsWithTheRows) {
  const StartMenuLayout eight = ComputeXpStartMenuLayout(96, 40, 100);
  const StartMenuLayout twelve = ComputeXpStartMenuLayout(96, 40, 100, 12, true);
  const StartMenuLayout small = ComputeXpStartMenuLayout(96, 40, 100, 8, false);
  EXPECT_EQ(twelve.listRows, 12);
  EXPECT_GT(twelve.frame.Height(), eight.frame.Height());
  EXPECT_EQ(small.rowHeight, 24);
  EXPECT_EQ(small.frame.Height(), eight.frame.Height());
}

}  // namespace
}  // namespace throwback
