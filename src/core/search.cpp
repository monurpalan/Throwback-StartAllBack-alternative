#include "core/search.h"

#include <vector>

namespace throwback {
namespace {

wchar_t FoldChar(wchar_t c) {
  switch (c) {
    case L'Á':
    case L'À':
    case L'Â':
    case L'Ä':
    case L'Ã':
    case L'Å':
    case L'á':
    case L'à':
    case L'â':
    case L'ä':
    case L'ã':
    case L'å':
      return L'a';
    case L'Ç':
    case L'ç':
      return L'c';
    case L'É':
    case L'È':
    case L'Ê':
    case L'Ë':
    case L'é':
    case L'è':
    case L'ê':
    case L'ë':
      return L'e';
    case L'Ğ':
    case L'ğ':
      return L'g';
    case L'Í':
    case L'Ì':
    case L'Î':
    case L'Ï':
    case L'İ':
    case L'í':
    case L'ì':
    case L'î':
    case L'ï':
    case L'ı':
      return L'i';
    case L'Ñ':
    case L'ñ':
      return L'n';
    case L'Ó':
    case L'Ò':
    case L'Ô':
    case L'Ö':
    case L'Õ':
    case L'ó':
    case L'ò':
    case L'ô':
    case L'ö':
    case L'õ':
      return L'o';
    case L'Ş':
    case L'ş':
      return L's';
    case L'Ú':
    case L'Ù':
    case L'Û':
    case L'Ü':
    case L'ú':
    case L'ù':
    case L'û':
    case L'ü':
      return L'u';
    case L'Ý':
    case L'ý':
    case L'ÿ':
      return L'y';
  }
  if (c >= L'A' && c <= L'Z') return static_cast<wchar_t>(c - L'A' + L'a');
  return c;
}

bool IsSeparator(wchar_t c) {
  return c == L' ' || c == L'-' || c == L'_' || c == L'.' || c == L'(' || c == L')' || c == L'&' ||
         c == L'/' || c == L'\\' || c == L',';
}

std::vector<std::wstring_view> SplitWords(std::wstring_view text) {
  std::vector<std::wstring_view> words;
  size_t start = 0;
  for (size_t i = 0; i <= text.size(); ++i) {
    if (i == text.size() || IsSeparator(text[i])) {
      if (i > start) words.push_back(text.substr(start, i - start));
      start = i + 1;
    }
  }
  return words;
}

}  // namespace

std::wstring FoldForSearch(std::wstring_view text) {
  std::wstring folded;
  folded.reserve(text.size());
  for (wchar_t c : text) folded.push_back(FoldChar(c));
  return folded;
}

int MatchScore(std::wstring_view name, std::wstring_view query) {
  const std::wstring foldedName = FoldForSearch(name);
  const std::wstring foldedQuery = FoldForSearch(query);
  const std::vector<std::wstring_view> queryWords = SplitWords(foldedQuery);
  if (queryWords.empty()) return 0;

  if (foldedName.starts_with(foldedQuery)) return 3;

  const std::vector<std::wstring_view> nameWords = SplitWords(foldedName);
  bool allWordsMatch = true;
  for (std::wstring_view queryWord : queryWords) {
    bool found = false;
    for (std::wstring_view nameWord : nameWords) {
      if (nameWord.starts_with(queryWord)) {
        found = true;
        break;
      }
    }
    allWordsMatch = allWordsMatch && found;
  }
  if (allWordsMatch) return 2;

  return foldedName.find(foldedQuery) != std::wstring::npos ? 1 : 0;
}

}  // namespace throwback
