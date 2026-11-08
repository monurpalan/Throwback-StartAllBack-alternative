#include "core/user_assist.h"

#include <gtest/gtest.h>

#include <cstring>
#include <vector>

namespace throwback {
namespace {

std::vector<std::uint8_t> MakeValue(std::uint32_t runCount, std::uint64_t lastRun) {
  std::vector<std::uint8_t> data(72, 0);
  std::memcpy(data.data() + 4, &runCount, sizeof(runCount));
  std::memcpy(data.data() + 60, &lastRun, sizeof(lastRun));
  return data;
}

TEST(UserAssistTest, Rot13RoundTripsAndKeepsOtherCharacters) {
  EXPECT_EQ(Rot13(L"Zvpebfbsg.Jvaqbjf.Rkcybere"), L"Microsoft.Windows.Explorer");
  EXPECT_EQ(Rot13(Rot13(L"{6D809377}\\notepad.exe!App")), L"{6D809377}\\notepad.exe!App");
}

TEST(UserAssistTest, ParsesRunCountAndLastRun) {
  const auto data = MakeValue(7, 133000000000000000ull);
  const auto entry = ParseUsageValue(Rot13(L"Microsoft.WindowsCalculator!App"), data);
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->appId, L"Microsoft.WindowsCalculator!App");
  EXPECT_EQ(entry->runCount, 7u);
  EXPECT_EQ(entry->lastRun, 133000000000000000ull);
}

TEST(UserAssistTest, SkipsBookkeepingShortAndUnusedValues) {
  EXPECT_FALSE(ParseUsageValue(Rot13(L"UEME_CTLSESSION"), MakeValue(3, 1)).has_value());
  EXPECT_FALSE(ParseUsageValue(Rot13(L"app.exe"), std::vector<std::uint8_t>(16, 0)).has_value());
  EXPECT_FALSE(ParseUsageValue(Rot13(L"app.exe"), MakeValue(0, 1)).has_value());
}

TEST(UserAssistTest, NewProgramsStayNewForAWeekUntilStarted) {
  constexpr std::uint64_t kDay = 24ull * 60 * 60 * 10'000'000;
  constexpr std::uint64_t kNow = 133000000000000000ull;
  EXPECT_TRUE(IsNewlyInstalled(kNow - kDay, kNow, 0));
  EXPECT_TRUE(IsNewlyInstalled(kNow - 7 * kDay, kNow, 0));
  EXPECT_FALSE(IsNewlyInstalled(kNow - 8 * kDay, kNow, 0));
  // Started after it was installed: no longer new. An older start (a
  // reinstall) doesn't count.
  EXPECT_FALSE(IsNewlyInstalled(kNow - kDay, kNow, kNow - kDay / 2));
  EXPECT_TRUE(IsNewlyInstalled(kNow - kDay, kNow, kNow - 30 * kDay));
  EXPECT_FALSE(IsNewlyInstalled(kNow + kDay, kNow, 0));
}

TEST(UserAssistTest, RecognizesJunkPrograms) {
  EXPECT_TRUE(IsJunkProgram(L"{7C5A40EF}\\Foo\\Setup.exe"));
  EXPECT_TRUE(IsJunkProgram(L"{7C5A40EF}\\Foo\\Uninstall Foo.lnk"));
  EXPECT_TRUE(IsJunkProgram(L"{1AC14E77}\\msiexec.exe"));
  EXPECT_FALSE(IsJunkProgram(L"{6D809377}\\Mozilla Firefox\\firefox.exe"));
}

TEST(UserAssistTest, RanksByCountThenRecencyAndDropsJunk) {
  const auto ranked = RankFrequent(
      {
          {L"a.exe", 3, 10},
          {L"setup.exe", 50, 99},
          {L"b.exe", 9, 1},
          {L"c.exe", 3, 20},
      },
      2);
  ASSERT_EQ(ranked.size(), 2u);
  EXPECT_EQ(ranked[0].appId, L"b.exe");
  EXPECT_EQ(ranked[1].appId, L"c.exe");
}

}  // namespace
}  // namespace throwback
