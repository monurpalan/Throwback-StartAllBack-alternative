#include "core/badge.h"

#include <cwctype>

namespace throwback {
namespace {

std::wstring_view ValueAttribute(std::wstring_view xml) {
  constexpr std::wstring_view kKey = L"value=";
  const size_t key = xml.find(kKey);
  if (key == std::wstring_view::npos || key + kKey.size() >= xml.size()) return {};
  const wchar_t quote = xml[key + kKey.size()];
  if (quote != L'"' && quote != L'\'') return {};
  const size_t start = key + kKey.size() + 1;
  const size_t end = xml.find(quote, start);
  if (end == std::wstring_view::npos) return {};
  return xml.substr(start, end - start);
}

}  // namespace

std::optional<Badge> ParseBadge(std::wstring_view xml) {
  const std::wstring_view value = ValueAttribute(xml);
  if (value.empty() || value == L"none") return std::nullopt;
  bool digits = true;
  for (wchar_t c : value) digits = digits && std::iswdigit(c);
  if (!digits) return Badge{.glyph = true};
  int count = 0;
  for (wchar_t c : value) count = count < 1000 ? count * 10 + (c - L'0') : count;
  if (count == 0) return std::nullopt;
  return Badge{.count = count};
}

std::wstring BadgeText(const Badge& badge) {
  if (badge.glyph) return {};
  return badge.count > 99 ? L"99+" : std::to_wstring(badge.count);
}

}  // namespace throwback
