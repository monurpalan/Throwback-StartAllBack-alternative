#include "core/window_menu.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

constexpr WindowMenuState kNormal = {.canMinimize = true, .canMaximize = true, .sizable = true};

TEST(WindowMenuTest, NormalWindowCanDoEverythingButRestore) {
  EXPECT_FALSE(IsCommandEnabled(WindowCommand::Restore, kNormal));
  for (WindowCommand command : {WindowCommand::Move, WindowCommand::Size, WindowCommand::Minimize,
                                WindowCommand::Maximize, WindowCommand::Close}) {
    EXPECT_TRUE(IsCommandEnabled(command, kNormal));
  }
  EXPECT_EQ(DefaultCommand(kNormal), WindowCommand::Close);
}

TEST(WindowMenuTest, MinimizedWindowCanBeRestoredButNotMovedOrSized) {
  WindowMenuState state = kNormal;
  state.minimized = true;
  EXPECT_TRUE(IsCommandEnabled(WindowCommand::Restore, state));
  EXPECT_FALSE(IsCommandEnabled(WindowCommand::Move, state));
  EXPECT_FALSE(IsCommandEnabled(WindowCommand::Size, state));
  EXPECT_FALSE(IsCommandEnabled(WindowCommand::Minimize, state));
  EXPECT_TRUE(IsCommandEnabled(WindowCommand::Maximize, state));
  EXPECT_EQ(DefaultCommand(state), WindowCommand::Restore);
}

TEST(WindowMenuTest, MaximizedWindowCannotMaximizeAgain) {
  WindowMenuState state = kNormal;
  state.maximized = true;
  EXPECT_TRUE(IsCommandEnabled(WindowCommand::Restore, state));
  EXPECT_FALSE(IsCommandEnabled(WindowCommand::Maximize, state));
  EXPECT_FALSE(IsCommandEnabled(WindowCommand::Size, state));
  EXPECT_TRUE(IsCommandEnabled(WindowCommand::Minimize, state));
}

TEST(WindowMenuTest, FixedDialogOnlyMovesAndCloses) {
  const WindowMenuState dialog{};
  EXPECT_TRUE(IsCommandEnabled(WindowCommand::Move, dialog));
  EXPECT_TRUE(IsCommandEnabled(WindowCommand::Close, dialog));
  EXPECT_FALSE(IsCommandEnabled(WindowCommand::Size, dialog));
  EXPECT_FALSE(IsCommandEnabled(WindowCommand::Minimize, dialog));
  EXPECT_FALSE(IsCommandEnabled(WindowCommand::Maximize, dialog));
}

}  // namespace
}  // namespace throwback
