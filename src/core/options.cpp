#include "core/options.h"

#include <algorithm>
#include <array>
#include <string_view>
#include <utility>

namespace throwback {
namespace {

constexpr std::wstring_view kProfilePrefix = L"--profile=";
constexpr std::wstring_view kStylePrefix = L"--start-style=";
constexpr std::array<std::pair<StartStyle, std::wstring_view>, 11> kStyles{{
    {StartStyle::Win98, L"98"},
    {StartStyle::WinMe, L"me"},
    {StartStyle::Win2000, L"2000"},
    {StartStyle::WinXp, L"xp"},
    {StartStyle::Vista, L"vista"},
    {StartStyle::Win7, L"7"},
    {StartStyle::Win8, L"8"},
    {StartStyle::Win81, L"8.1"},
    {StartStyle::Win10, L"10"},
    {StartStyle::Win11, L"11"},
    {StartStyle::Plain8, L"plain8"},
}};
constexpr size_t kMaxProfileLength = 32;

// Applies one known argument; returns false for anything unknown.
bool Apply(std::wstring_view arg, Options& options) {
  if (arg.starts_with(kProfilePrefix)) {
    const std::wstring_view name = arg.substr(kProfilePrefix.size());
    if (!IsValidProfileName(name)) return false;
    options.profile = name;
  } else if (arg.starts_with(kStylePrefix)) {
    options.startStyle = ParseStartStyle(arg.substr(kStylePrefix.size()));
    if (!options.startStyle) return false;
  } else if (arg == L"--edge=top") {
    options.edge = Edge::Top;
  } else if (arg == L"--edge=bottom") {
    options.edge = Edge::Bottom;
  } else if (arg == L"--edge=left") {
    options.edge = Edge::Left;
  } else if (arg == L"--edge=right") {
    options.edge = Edge::Right;
  } else if (arg == L"--theme=system") {
    options.theme = ThemeMode::System;
  } else if (arg == L"--theme=light") {
    options.theme = ThemeMode::Light;
  } else if (arg == L"--theme=dark") {
    options.theme = ThemeMode::Dark;
  } else if (arg == L"--lang=system") {
    options.language = LanguageMode::System;
  } else if (arg == L"--lang=en") {
    options.language = LanguageMode::English;
  } else if (arg == L"--lang=tr") {
    options.language = LanguageMode::Turkish;
  } else {
    return false;
  }
  return true;
}

}  // namespace

std::wstring_view StartStyleName(StartStyle style) {
  for (const auto& [item, name] : kStyles) {
    if (item == style) return name;
  }
  return {};
}

std::optional<StartStyle> ParseStartStyle(std::wstring_view name) {
  for (const auto& [item, styleName] : kStyles) {
    if (styleName == name) return item;
  }
  return std::nullopt;
}

bool IsValidProfileName(std::wstring_view name) {
  if (name.empty() || name.size() > kMaxProfileLength) return false;
  return std::ranges::all_of(name, [](wchar_t c) {
    return (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9') ||
           c == L'-' || c == L'_';
  });
}

ParseResult ParseOptions(const std::vector<std::wstring>& args) {
  ParseResult result;
  for (const auto& arg : args) {
    if (!Apply(arg, result.options)) result.unknownArguments.push_back(arg);
  }
  return result;
}

}  // namespace throwback
