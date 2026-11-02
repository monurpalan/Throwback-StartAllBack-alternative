#include "core/options.h"

namespace throwback {

ParseResult ParseOptions(const std::vector<std::wstring>& args) {
  ParseResult result;
  for (const auto& arg : args) {
    if (arg == L"--edge=top") {
      result.options.edge = Edge::Top;
    } else if (arg == L"--edge=bottom") {
      result.options.edge = Edge::Bottom;
    } else if (arg == L"--theme=system") {
      result.options.theme = ThemeMode::System;
    } else if (arg == L"--theme=light") {
      result.options.theme = ThemeMode::Light;
    } else if (arg == L"--theme=dark") {
      result.options.theme = ThemeMode::Dark;
    } else {
      result.errors.push_back(L"Unknown argument: " + arg);
    }
  }
  return result;
}

}  // namespace throwback
