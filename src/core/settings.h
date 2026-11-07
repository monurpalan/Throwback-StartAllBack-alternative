#pragma once

#include <map>
#include <string>
#include <vector>

#include "core/options.h"

namespace throwback {

// Everything the user can change, saved per profile.
struct Settings {
  Edge edge = Edge::Top;
  ThemeMode theme = ThemeMode::System;
  LanguageMode language = LanguageMode::System;
  // Hides Explorer's taskbar while Throwback runs and brings it back on exit.
  bool hideSystemTaskbar = false;
  // A lone press of the Windows key opens our start menu instead of Windows'.
  bool windowsKeyOpensMenu = true;
  // Pinned start menu programs, as shell parsing names, in display order.
  std::vector<std::wstring> pinned;
  // Notification icons the user dragged onto the bar or off it, by icon
  // identity. Icons in neither list follow Windows' own choice.
  std::vector<std::wstring> trayShown;
  std::vector<std::wstring> trayHidden;

  bool operator==(const Settings&) const = default;
};

// Settings as named text values, the form they are stored in. Reading is
// forgiving: a missing or unknown value keeps its default.
using SettingValues = std::map<std::wstring, std::wstring>;
SettingValues ToValues(const Settings& settings);
Settings FromValues(const SettingValues& values);

// The settings for this run: the saved ones with command-line overrides.
Settings WithOverrides(Settings settings, const Options& options);

// Pinning keeps one entry per program; unpinning a missing one does nothing.
void Pin(Settings& settings, const std::wstring& target);
void Unpin(Settings& settings, const std::wstring& target);
bool IsPinned(const Settings& settings, const std::wstring& target);

// Whether a notification icon sits on the bar or behind the arrow.
bool IsTrayIconShown(const Settings& settings, const std::wstring& identity, bool windowsShows);
void SetTrayIconShown(Settings& settings, const std::wstring& identity, bool shown);

}  // namespace throwback
