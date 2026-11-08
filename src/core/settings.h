#pragma once

#include <map>
#include <string>
#include <vector>

#include "core/options.h"

namespace throwback {

// Which side of the screen the start menu hugs. Vertically it follows the
// bar unless pinned to the top or bottom of the screen.
enum class MenuHorizontal { Left, Right };
enum class MenuVertical { Bar, Top, Bottom };
// The start button of the Windows 7 style menus: the style's own, or one of
// the orbs StartAllBack ships.
enum class OrbChoice { Style, Win7, Vista, Clover, Win8, E1even };

// The fewest and most program rows the start menu shows.
constexpr int kMinMenuRows = 4;
constexpr int kMaxMenuRows = 20;
constexpr int kDefaultMenuRows = 10;

// Whether a style's start menu has a program list and a right column the
// Customize Start Menu dialog can shape (XP, Vista, 7 and Plain8). The
// classic cascading menus and the tiled ones of 8 to 11 have neither.
inline bool HasCustomizableMenu(StartStyle style) {
  return style == StartStyle::WinXp || style == StartStyle::Vista || style == StartStyle::Win7 ||
         style == StartStyle::Plain8;
}

// Everything the user can change, saved per profile.
struct Settings {
  Edge edge = Edge::Top;
  ThemeMode theme = ThemeMode::System;
  LanguageMode language = LanguageMode::System;
  // Hides Explorer's taskbar while Throwback runs and brings it back on exit.
  bool hideSystemTaskbar = false;
  // A lone press of the Windows key opens our start menu instead of Windows'.
  bool windowsKeyOpensMenu = true;
  // Throwback starts with the user's session (the Run key), as StartAllBack
  // does by loading with Explorer.
  bool startWithWindows = true;
  // Set once the first-run setup ran: Throwback took the Windows taskbar's
  // place and showed its properties (StartAllBack's WelcomeShown).
  bool welcomeShown = false;
  // The Search and Task View buttons beside the Start button, and whether
  // the bar is locked (unlocked, it can be dragged to another edge).
  bool showSearchButton = true;
  bool showTaskViewButton = true;
  bool taskbarLocked = true;
  StartStyle startStyle = StartStyle::Win7;
  MenuHorizontal menuHorizontal = MenuHorizontal::Left;
  MenuVertical menuVertical = MenuVertical::Bar;
  OrbChoice orb = OrbChoice::Style;
  // Program rows of the Windows 7 style menus, and whether they use large
  // (32 px) or small (16 px) icons.
  int menuRows = kDefaultMenuRows;
  bool largeIcons = true;
  // Right column places the user turned on or off, by key (see
  // startmenu/places.h). Places in neither list keep their default.
  std::vector<std::wstring> placesShown;
  std::vector<std::wstring> placesHidden;
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

// Whether a right column place is shown.
bool IsPlaceShown(const Settings& settings, const std::wstring& key, bool shownByDefault);
void SetPlaceShown(Settings& settings, const std::wstring& key, bool shown);

// Whether a notification icon sits on the bar or behind the arrow.
bool IsTrayIconShown(const Settings& settings, const std::wstring& identity, bool windowsShows);
void SetTrayIconShown(Settings& settings, const std::wstring& identity, bool shown);

}  // namespace throwback
