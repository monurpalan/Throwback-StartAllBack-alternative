#include "core/program_menu.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

std::vector<ProgramCommand> Commands(bool fileTarget, bool pinned) {
  std::vector<ProgramCommand> commands;
  for (const auto& item : ProgramMenuItems(fileTarget, pinned)) commands.push_back(item.command);
  return commands;
}

TEST(ProgramMenuTest, RecognizesFileTargets) {
  EXPECT_TRUE(IsFileTarget(L"C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\a.lnk"));
  EXPECT_TRUE(IsFileTarget(L"\\\\server\\share\\a.exe"));
  EXPECT_FALSE(IsFileTarget(L"shell:AppsFolder\\Microsoft.WindowsCalculator!App"));
  EXPECT_FALSE(IsFileTarget(L"ms-settings:display"));
  EXPECT_FALSE(IsFileTarget(L""));
}

TEST(ProgramMenuTest, ShortcutsGetEveryCommand) {
  using enum ProgramCommand;
  EXPECT_EQ(Commands(true, false),
            (std::vector{Open, RunAsAdministrator, OpenFileLocation, Pin, Properties}));
}

TEST(ProgramMenuTest, PackagedAppsSkipFileCommands) {
  using enum ProgramCommand;
  EXPECT_EQ(Commands(false, false), (std::vector{Open, Pin}));
}

TEST(ProgramMenuTest, PinnedProgramsOfferUnpin) {
  using enum ProgramCommand;
  EXPECT_EQ(Commands(false, true), (std::vector{Open, Unpin}));
  const auto items = ProgramMenuItems(true, true);
  EXPECT_FALSE(items.front().separatorBefore);
  EXPECT_TRUE(items[3].separatorBefore);
}

}  // namespace
}  // namespace throwback
