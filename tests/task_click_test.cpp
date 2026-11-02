#include "core/task_click.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

TEST(TaskClickTest, BackgroundWindowIsActivated) {
  EXPECT_EQ(DecideTaskClick({.wasActive = false, .canMinimize = true}), TaskClickAction::Activate);
}

TEST(TaskClickTest, ActiveWindowIsMinimized) {
  EXPECT_EQ(DecideTaskClick({.wasActive = true, .canMinimize = true}), TaskClickAction::Minimize);
}

TEST(TaskClickTest, MinimizedWindowIsRestoredEvenIfItWasActive) {
  EXPECT_EQ(DecideTaskClick({.wasActive = true, .minimized = true}), TaskClickAction::Restore);
  EXPECT_EQ(DecideTaskClick({.minimized = true}), TaskClickAction::Restore);
}

TEST(TaskClickTest, ActiveWindowThatCannotMinimizeIsLeftAlone) {
  // No minimize box, or a modal dialog has disabled the window.
  EXPECT_EQ(DecideTaskClick({.wasActive = true, .canMinimize = false}), TaskClickAction::None);
}

}  // namespace
}  // namespace throwback
