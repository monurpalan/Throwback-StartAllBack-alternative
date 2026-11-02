#include "core/options.h"

#include <string_view>

namespace throwback {
namespace {

// Applies one known argument; returns false for anything unknown.
bool Apply(std::wstring_view arg, Options& options) {
  if (arg == L"--edge=top") {
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

ParseResult ParseOptions(const std::vector<std::wstring>& args) {
  ParseResult result;
  for (const auto& arg : args) {
    if (!Apply(arg, result.options)) result.unknownArguments.push_back(arg);
  }
  return result;
}

}  // namespace throwback
