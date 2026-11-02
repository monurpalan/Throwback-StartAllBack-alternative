#include "core/task_progress.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

TEST(TaskProgressTest, MapsFlags) {
  EXPECT_EQ(ProgressStateFromFlags(0), ProgressState::None);
  EXPECT_EQ(ProgressStateFromFlags(0x1), ProgressState::Indeterminate);
  EXPECT_EQ(ProgressStateFromFlags(0x2), ProgressState::Normal);
  EXPECT_EQ(ProgressStateFromFlags(0x4), ProgressState::Error);
  EXPECT_EQ(ProgressStateFromFlags(0x8), ProgressState::Paused);
}

TEST(TaskProgressTest, ScalesAndClampsValues) {
  EXPECT_FLOAT_EQ(ProgressFraction(0), 0.0f);
  EXPECT_FLOAT_EQ(ProgressFraction(32767), 32767.0f / 65534.0f);
  EXPECT_FLOAT_EQ(ProgressFraction(65534), 1.0f);
  EXPECT_FLOAT_EQ(ProgressFraction(70000), 1.0f);
  EXPECT_FLOAT_EQ(ProgressFraction(-5), 0.0f);
}

}  // namespace
}  // namespace throwback
