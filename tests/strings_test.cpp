#include "ui/strings.h"

#include <gtest/gtest.h>

#include <set>
#include <string>

namespace throwback::ui {
namespace {

TEST(StringsTest, EveryStringHasEnglishAndTurkish) {
  const Translation* table = Translations();
  for (std::size_t i = 0; i < kStringCount; ++i) {
    EXPECT_EQ(static_cast<std::size_t>(table[i].id), i);
    EXPECT_FALSE(table[i].english.empty()) << "string " << i;
    EXPECT_FALSE(table[i].turkish.empty()) << "string " << i;
  }
}

TEST(StringsTest, EnglishTextsAreUnique) {
  std::set<std::wstring> seen;
  const Translation* table = Translations();
  for (std::size_t i = 0; i < kStringCount; ++i) {
    EXPECT_TRUE(seen.insert(std::wstring(table[i].english)).second) << "string " << i;
  }
}

TEST(StringsTest, FollowsSelectedLanguage) {
  SetLanguage(Language::Turkish);
  EXPECT_EQ(Tr(StringId::AllPrograms), L"Tüm Programlar");
  SetLanguage(Language::English);
  EXPECT_EQ(Tr(StringId::AllPrograms), L"All Programs");
}

}  // namespace
}  // namespace throwback::ui
