#pragma once

#include <string>
#include <string_view>

namespace throwback {

// Lower-cases and strips accents, including Turkish letters, so "calistir"
// finds "Çalıştır" and "KAPAT" finds "kapat".
std::wstring FoldForSearch(std::wstring_view text);

// How well a program name matches what the user typed; 0 means no match.
// 3: the name starts with the query; 2: every query word starts a word of
// the name; 1: the name contains the query.
int MatchScore(std::wstring_view name, std::wstring_view query);

}  // namespace throwback
