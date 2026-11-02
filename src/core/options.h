#pragma once

#include <string>
#include <vector>

namespace throwback {

enum class Edge { Top, Bottom };
enum class ThemeMode { System, Light, Dark };

struct Options {
  Edge edge = Edge::Top;
  ThemeMode theme = ThemeMode::System;
};

struct ParseResult {
  Options options;
  std::vector<std::wstring> errors;
};

// Parses command-line arguments (without the program name).
ParseResult ParseOptions(const std::vector<std::wstring>& args);

}  // namespace throwback
