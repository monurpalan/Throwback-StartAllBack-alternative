#include "core/volume_math.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

TEST(VolumeMathTest, GlyphFollowsLevelAndMute) {
  EXPECT_EQ(VolumeGlyphStep(0.5f, true), 0);
  EXPECT_EQ(VolumeGlyphStep(0.0f, false), 0);
  EXPECT_EQ(VolumeGlyphStep(0.2f, false), 1);
  EXPECT_EQ(VolumeGlyphStep(0.5f, false), 2);
  EXPECT_EQ(VolumeGlyphStep(0.9f, false), 3);
}

TEST(VolumeMathTest, SliderMapsTopToFull) {
  EXPECT_FLOAT_EQ(LevelAtY(100, 100, 200), 1.0f);
  EXPECT_FLOAT_EQ(LevelAtY(200, 100, 200), 0.0f);
  EXPECT_FLOAT_EQ(LevelAtY(150, 100, 200), 0.5f);
  EXPECT_FLOAT_EQ(LevelAtY(50, 100, 200), 1.0f);  // above the track
  EXPECT_EQ(ThumbY(0.25f, 100, 200), 175);
  EXPECT_EQ(ThumbY(LevelAtY(130, 100, 200), 100, 200), 130);
}

TEST(VolumeMathTest, WheelStepsTwoPercent) {
  EXPECT_FLOAT_EQ(WheelLevel(0.50f, 120), 0.52f);
  EXPECT_FLOAT_EQ(WheelLevel(0.50f, -240), 0.46f);
  EXPECT_FLOAT_EQ(WheelLevel(0.99f, 120), 1.0f);
  EXPECT_FLOAT_EQ(WheelLevel(0.01f, -120), 0.0f);
  EXPECT_EQ(VolumePercent(0.456f), 46);
}

}  // namespace
}  // namespace throwback
