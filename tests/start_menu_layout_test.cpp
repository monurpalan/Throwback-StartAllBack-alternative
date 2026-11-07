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
  EXPECT_EQ(layout.frame, (Rect{0, 0, 400, 518}));
  EXPECT_EQ(layout.leftPane, (Rect{8, 8, 256, 464}));
  EXPECT_EQ(layout.list, (Rect{8, 8, 256, 408}));
  EXPECT_EQ(layout.allPrograms, (Rect{8, 424, 256, 464}));
  EXPECT_EQ(layout.search, (Rect{21, 476, 238, 498}));
  EXPECT_EQ(layout.picture, (Rect{304, 8, 352, 56}));
  EXPECT_EQ(layout.power, (Rect{264, 475, 340, 499}));
  EXPECT_EQ(layout.powerArrow, (Rect{340, 475, 364, 499}));
  EXPECT_EQ(layout.jumpList, (Rect{264, 8, 392, 464}));
  EXPECT_EQ(layout.programsArea, (Rect{0, 0, 256, 464}));
  EXPECT_EQ(layout.searchArea, (Rect{0, 464, 256, 518}));
  EXPECT_EQ(layout.placesArea, (Rect{256, 0, 400, 464}));
  EXPECT_EQ(layout.powerArea, (Rect{256, 464, 400, 518}));
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

TEST(StartMenuLayoutTest, ScalesWithDpi) {
  EXPECT_EQ(ComputeStartMenuLayout(192).frame, (Rect{0, 0, 800, 1036}));
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

}  // namespace
}  // namespace throwback
