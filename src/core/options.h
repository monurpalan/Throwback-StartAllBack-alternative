#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace throwback {

enum class Edge { Top, Bottom };
enum class ThemeMode { System, Light, Dark };
enum class LanguageMode { System, English, Turkish };
// Which Windows version's start menu (and start button) to show.
enum class StartStyle { Win98, WinMe, Win2000, WinXp, Vista, Win7, Win8, Win81, Win10, Win11 };

// The short names used on the command line and in saved settings: "98",
// "me", "2000", "xp", "vista", "7", "8", "8.1", "10", "11".
std::wstring_view StartStyleName(StartStyle style);
std::optional<StartStyle> ParseStartStyle(std::wstring_view name);

// Command-line arguments. They override the saved settings for one run and
// are never saved themselves.
struct Options {
  std::optional<Edge> edge;
  std::optional<ThemeMode> theme;
  std::optional<LanguageMode> language;
  std::optional<StartStyle> startStyle;
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
