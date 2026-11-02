#include "core/monitor_diff.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

using Names = std::vector<std::wstring>;

TEST(MonitorDiffTest, NothingChanges) {
  const MonitorDiff diff = DiffMonitors({L"A", L"B"}, {L"B", L"A"});
  EXPECT_TRUE(diff.added.empty());
  EXPECT_TRUE(diff.removed.empty());
}

TEST(MonitorDiffTest, PlugAndUnplug) {
  const MonitorDiff diff = DiffMonitors({L"A", L"B"}, {L"B", L"C", L"D"});
  EXPECT_EQ(diff.added, (Names{L"C", L"D"}));
  EXPECT_EQ(diff.removed, (Names{L"A"}));
}

TEST(MonitorDiffTest, FromNothing) {
  EXPECT_EQ(DiffMonitors({}, {L"A"}).added, (Names{L"A"}));
  EXPECT_EQ(DiffMonitors({L"A"}, {}).removed, (Names{L"A"}));
}

}  // namespace
}  // namespace throwback
