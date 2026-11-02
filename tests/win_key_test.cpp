#include "core/win_key.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

constexpr unsigned kWin = 0x5B;
constexpr unsigned kRightWin = 0x5C;
constexpr unsigned kE = 'E';
constexpr unsigned kCtrl = 0xA2;
constexpr unsigned kEscape = 0x1B;

TEST(WinKeyTest, TapOpensTheMenu) {
  WinKeyTracker tracker;
  EXPECT_EQ(tracker.OnKey(kWin, true), WinKeyAction::PassThrough);
  EXPECT_EQ(tracker.OnKey(kWin, false), WinKeyAction::OpenMenuInsteadOfRelease);
}

TEST(WinKeyTest, KeyRepeatIsStillATap) {
  WinKeyTracker tracker;
  tracker.OnKey(kRightWin, true);
  tracker.OnKey(kRightWin, true);
  EXPECT_EQ(tracker.OnKey(kRightWin, false), WinKeyAction::OpenMenuInsteadOfRelease);
}

TEST(WinKeyTest, CombinationsAreLeftToWindows) {
  WinKeyTracker tracker;
  tracker.OnKey(kWin, true);
  EXPECT_EQ(tracker.OnKey(kE, true), WinKeyAction::PassThrough);
  EXPECT_EQ(tracker.OnKey(kE, false), WinKeyAction::PassThrough);
  EXPECT_EQ(tracker.OnKey(kWin, false), WinKeyAction::PassThrough);

  // The next plain tap works again.
  tracker.OnKey(kWin, true);
  EXPECT_EQ(tracker.OnKey(kWin, false), WinKeyAction::OpenMenuInsteadOfRelease);
}

TEST(WinKeyTest, ReleaseWithoutPressDoesNothing) {
  WinKeyTracker tracker;
  EXPECT_EQ(tracker.OnKey(kWin, false), WinKeyAction::PassThrough);
}

TEST(WinKeyTest, CtrlEscapeOpensTheMenu) {
  WinKeyTracker tracker;
  tracker.OnKey(kCtrl, true);
  EXPECT_EQ(tracker.OnKey(kEscape, true), WinKeyAction::OpenMenuAndSwallow);
  tracker.OnKey(kCtrl, false);
  EXPECT_EQ(tracker.OnKey(kEscape, true), WinKeyAction::PassThrough);
}

}  // namespace
}  // namespace throwback
