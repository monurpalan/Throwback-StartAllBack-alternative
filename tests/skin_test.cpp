#include "ui/skin.h"

#include <gtest/gtest.h>

#include "ui/skin_ids.h"

namespace throwback::ui {
namespace {

TEST(SkinTest, PicksTheFileForTheDpi) {
  const SkinPart& orb = PartOf(SkinImage::Orb);
  EXPECT_EQ(ResourceFor(orb, 96), THROWBACK_SKIN_ORB_96);
  EXPECT_EQ(ResourceFor(orb, 120), THROWBACK_SKIN_ORB_120);
  EXPECT_EQ(ResourceFor(orb, 132), THROWBACK_SKIN_ORB_144);
  EXPECT_EQ(ResourceFor(orb, 144), THROWBACK_SKIN_ORB_144);
  EXPECT_EQ(ResourceFor(orb, 192), THROWBACK_SKIN_ORB_192);
  EXPECT_EQ(ResourceFor(orb, 288), THROWBACK_SKIN_ORB_192);
}

TEST(SkinTest, SingleFileImagesIgnoreTheDpi) {
  const SkinPart& button = PartOf(SkinImage::TaskButton);
  EXPECT_EQ(ResourceFor(button, 96), THROWBACK_SKIN_TASK_BUTTON);
  EXPECT_EQ(ResourceFor(button, 192), THROWBACK_SKIN_TASK_BUTTON);
}

TEST(SkinTest, EveryImageHasAFrameAndAFile) {
  for (int i = 0; i <= static_cast<int>(SkinImage::SplitArrow); ++i) {
    const SkinPart& part = PartOf(static_cast<SkinImage>(i));
    EXPECT_GT(part.resource, 0) << i;
    EXPECT_GE(part.frames, 1) << i;
  }
}

}  // namespace
}  // namespace throwback::ui
