#include "core/options.h"

#include <algorithm>
#include <string_view>

namespace throwback {
namespace {

constexpr std::wstring_view kProfilePrefix = L"--profile=";
constexpr size_t kMaxProfileLength = 32;

// Applies one known argument; returns false for anything unknown.
bool Apply(std::wstring_view arg, Options& options) {
  if (arg.starts_with(kProfilePrefix)) {
    const std::wstring_view name = arg.substr(kProfilePrefix.size());
    if (!IsValidProfileName(name)) return false;
    options.profile = name;
  } else if (arg == L"--edge=top") {
    options.edge = Edge::Top;
  } else if (arg == L"--edge=bottom") {
    options.edge = Edge::Bottom;
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
