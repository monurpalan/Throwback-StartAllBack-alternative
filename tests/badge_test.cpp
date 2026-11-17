#include "core/badge.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

TEST(BadgeTest, ReadsCountsAndGlyphs) {
  EXPECT_EQ(ParseBadge(L"<badge value=\"1\"/>"), (Badge{.count = 1}));
  EXPECT_EQ(ParseBadge(L"<badge version='1' value='12'/>"), (Badge{.count = 12}));
  EXPECT_EQ(ParseBadge(L"<badge value=\"newMessage\"/>"), (Badge{.glyph = true}));
}

TEST(BadgeTest, NothingForNoneZeroOrJunk) {
  EXPECT_FALSE(ParseBadge(L"<badge value=\"0\"/>"));
  EXPECT_FALSE(ParseBadge(L"<badge value=\"none\"/>"));
  EXPECT_FALSE(ParseBadge(L"<badge/>"));
  EXPECT_FALSE(ParseBadge(L""));
}

TEST(BadgeTest, TextCapsAtNinetyNine) {
  EXPECT_EQ(BadgeText({.count = 7}), L"7");
  EXPECT_EQ(BadgeText({.count = 120}), L"99+");
  EXPECT_EQ(BadgeText({.glyph = true}), L"");
}

}  // namespace
}  // namespace throwback
