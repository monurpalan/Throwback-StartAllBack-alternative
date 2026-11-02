#include "core/search.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

TEST(SearchTest, FoldsCaseAndTurkishLetters) {
  EXPECT_EQ(FoldForSearch(L"Çalıştır"), L"calistir");
  EXPECT_EQ(FoldForSearch(L"İŞĞÜÖ"), L"isguo");
  EXPECT_EQ(FoldForSearch(L"Notepad++"), L"notepad++");
}

TEST(SearchTest, PrefixOfWholeNameScoresHighest) {
  EXPECT_EQ(MatchScore(L"Notepad++", L"note"), 3);
  EXPECT_EQ(MatchScore(L"Hesap Makinesi", L"hesap m"), 3);
}

TEST(SearchTest, WordPrefixesMatchInAnyOrder) {
  EXPECT_EQ(MatchScore(L"Visual Studio Code", L"code"), 2);
  EXPECT_EQ(MatchScore(L"Visual Studio Code", L"code vis"), 2);
  EXPECT_EQ(MatchScore(L"Microsoft Edge", L"edge"), 2);
}

TEST(SearchTest, SubstringIsTheWeakestMatch) { EXPECT_EQ(MatchScore(L"PowerShell", L"shell"), 1); }

TEST(SearchTest, AccentsAndCaseDoNotMatter) {
  EXPECT_EQ(MatchScore(L"Görev Yöneticisi", L"GOREV"), 3);
  EXPECT_EQ(MatchScore(L"Denetim Masası", L"masasi"), 2);
}

TEST(SearchTest, NonMatchesAndEmptyQueries) {
  EXPECT_EQ(MatchScore(L"Paint", L"word"), 0);
  EXPECT_EQ(MatchScore(L"Paint", L""), 0);
  EXPECT_EQ(MatchScore(L"Paint", L"   "), 0);
  EXPECT_EQ(MatchScore(L"Visual Studio", L"studio xyz"), 0);
}

}  // namespace
}  // namespace throwback
