#pragma once

#include <string>
#include <vector>

namespace throwback {

enum class Edge { Top, Bottom };
enum class ThemeMode { System, Light, Dark };
enum class LanguageMode { System, English, Turkish };

struct Options {
  Edge edge = Edge::Top;
  ThemeMode theme = ThemeMode::System;
  LanguageMode language = LanguageMode::System;
};

struct ParseResult {
  Options options;
  // Reported to the user by the caller, in the user's language.
  std::vector<std::wstring> unknownArguments;
};

// Parses command-line arguments (without the program name).
ParseResult ParseOptions(const std::vector<std::wstring>& args);

}  // namespace throwback
