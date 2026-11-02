#pragma once

#include <optional>
#include <string>
#include <vector>

namespace throwback {

enum class Edge { Top, Bottom };
enum class ThemeMode { System, Light, Dark };
enum class LanguageMode { System, English, Turkish };

// Command-line arguments. They override the saved settings for one run and
// are never saved themselves.
struct Options {
  std::optional<Edge> edge;
  std::optional<ThemeMode> theme;
  std::optional<LanguageMode> language;
  // Settings live per profile; tests run under their own so they never touch
  // the user's settings.
  std::wstring profile = L"default";
};

struct ParseResult {
  Options options;
  // Reported to the user by the caller, in the user's language.
  std::vector<std::wstring> unknownArguments;
};

// Parses command-line arguments (without the program name).
ParseResult ParseOptions(const std::vector<std::wstring>& args);

// Letters, digits, '-' and '_', at most 32 characters: safe as a registry key.
bool IsValidProfileName(std::wstring_view name);

}  // namespace throwback
