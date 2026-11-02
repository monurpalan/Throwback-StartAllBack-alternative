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
  EXPECT_EQ(layout.frame, (Rect{0, 0, 400, 512}));
  EXPECT_EQ(layout.leftPane, (Rect{8, 8, 256, 464}));
  EXPECT_EQ(layout.list, (Rect{8, 8, 256, 408}));
  EXPECT_EQ(layout.allPrograms, (Rect{8, 424, 256, 464}));
  EXPECT_EQ(layout.search, (Rect{16, 476, 248, 500}));
  EXPECT_EQ(layout.picture, (Rect{304, 8, 352, 56}));
  EXPECT_EQ(layout.power, (Rect{264, 476, 368, 500}));
  EXPECT_EQ(layout.powerArrow, (Rect{368, 476, 392, 500}));
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
  }
}

TEST(StartMenuLayoutTest, ScalesWithDpi) {
  EXPECT_EQ(ComputeStartMenuLayout(192).frame, (Rect{0, 0, 800, 1024}));
  EXPECT_EQ(ComputeStartMenuLayout(192).rowHeight, 80);
}

}  // namespace
}  // namespace throwback
