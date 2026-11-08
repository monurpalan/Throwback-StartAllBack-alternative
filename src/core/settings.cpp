#include "core/settings.h"

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <utility>

namespace throwback {
namespace {

template <typename Enum, size_t N>
using Names = std::array<std::pair<Enum, std::wstring_view>, N>;

constexpr Names<Edge, 4> kEdges{{{Edge::Top, L"top"},
                                 {Edge::Bottom, L"bottom"},
                                 {Edge::Left, L"left"},
                                 {Edge::Right, L"right"}}};
constexpr Names<ThemeMode, 3> kThemes{
    {{ThemeMode::System, L"system"}, {ThemeMode::Light, L"light"}, {ThemeMode::Dark, L"dark"}}};
constexpr Names<MenuHorizontal, 2> kHorizontals{
    {{MenuHorizontal::Left, L"left"}, {MenuHorizontal::Right, L"right"}}};
constexpr Names<MenuVertical, 3> kVerticals{
    {{MenuVertical::Bar, L"bar"}, {MenuVertical::Top, L"top"}, {MenuVertical::Bottom, L"bottom"}}};
constexpr Names<OrbChoice, 6> kOrbs{{{OrbChoice::Style, L"style"},
                                     {OrbChoice::Win7, L"7"},
                                     {OrbChoice::Vista, L"vista"},
                                     {OrbChoice::Clover, L"clover"},
                                     {OrbChoice::Win8, L"8"},
                                     {OrbChoice::E1even, L"e1even"}}};
constexpr Names<LanguageMode, 3> kLanguages{{{LanguageMode::System, L"system"},
                                             {LanguageMode::English, L"en"},
                                             {LanguageMode::Turkish, L"tr"}}};

template <typename Enum, size_t N>
std::wstring NameOf(const Names<Enum, N>& names, Enum value) {
  for (const auto& [item, name] : names) {
    if (item == value) return std::wstring(name);
  }
  return {};
}

template <typename Enum, size_t N>
void ReadEnum(const SettingValues& values, const wchar_t* key, const Names<Enum, N>& names,
              Enum& out) {
  const auto it = values.find(key);
  if (it == values.end()) return;
  for (const auto& [item, name] : names) {
    if (it->second == name) out = item;
  }
}

void ReadBool(const SettingValues& values, const wchar_t* key, bool& out) {
  const auto it = values.find(key);
  if (it == values.end()) return;
  if (it->second == L"1") out = true;
  if (it->second == L"0") out = false;
}

void ReadRows(const SettingValues& values, int& out) {
  const auto it = values.find(L"MenuRows");
  if (it == values.end() || it->second.empty() || it->second.size() > 2) return;
  int rows = 0;
  for (wchar_t c : it->second) {
    if (c < L'0' || c > L'9') return;
    rows = rows * 10 + (c - L'0');
  }
  if (rows >= kMinMenuRows && rows <= kMaxMenuRows) out = rows;
}

// Moves key into the chosen one of two override lists.
void SetOverride(std::vector<std::wstring>& on, std::vector<std::wstring>& off,
                 const std::wstring& key, bool shown) {
  if (key.empty()) return;
  std::erase(on, key);
  std::erase(off, key);
  (shown ? on : off).push_back(key);
}

bool IsOverridden(const std::vector<std::wstring>& on, const std::vector<std::wstring>& off,
                  const std::wstring& key, bool byDefault) {
  if (std::ranges::find(on, key) != on.end()) return true;
  if (std::ranges::find(off, key) != off.end()) return false;
  return byDefault;
}

std::wstring JoinLines(const std::vector<std::wstring>& lines) {
  std::wstring text;
  for (const auto& line : lines) {
    if (!text.empty()) text += L'\n';
    text += line;
  }
  return text;
}

std::vector<std::wstring> SplitLines(std::wstring_view text) {
  std::vector<std::wstring> lines;
  while (!text.empty()) {
    const size_t end = text.find(L'\n');
    const std::wstring_view line = text.substr(0, end);
    if (!line.empty()) lines.emplace_back(line);
    if (end == std::wstring_view::npos) break;
    text.remove_prefix(end + 1);
  }
  return lines;
}

}  // namespace

SettingValues ToValues(const Settings& settings) {
  return {
      {L"Edge", NameOf(kEdges, settings.edge)},
      {L"Theme", NameOf(kThemes, settings.theme)},
      {L"Language", NameOf(kLanguages, settings.language)},
      {L"HideSystemTaskbar", settings.hideSystemTaskbar ? L"1" : L"0"},
      {L"WindowsKeyOpensMenu", settings.windowsKeyOpensMenu ? L"1" : L"0"},
      {L"StartWithWindows", settings.startWithWindows ? L"1" : L"0"},
      {L"WelcomeShown", settings.welcomeShown ? L"1" : L"0"},
      {L"ShowSearchButton", settings.showSearchButton ? L"1" : L"0"},
      {L"ShowTaskViewButton", settings.showTaskViewButton ? L"1" : L"0"},
      {L"TaskbarLocked", settings.taskbarLocked ? L"1" : L"0"},
      {L"StartStyle", std::wstring(StartStyleName(settings.startStyle))},
      {L"MenuHorizontal", NameOf(kHorizontals, settings.menuHorizontal)},
      {L"MenuVertical", NameOf(kVerticals, settings.menuVertical)},
      {L"Orb", NameOf(kOrbs, settings.orb)},
      {L"MenuRows", std::to_wstring(settings.menuRows)},
      {L"LargeIcons", settings.largeIcons ? L"1" : L"0"},
      {L"PlacesShown", JoinLines(settings.placesShown)},
      {L"PlacesHidden", JoinLines(settings.placesHidden)},
      {L"Pinned", JoinLines(settings.pinned)},
      {L"TrayShown", JoinLines(settings.trayShown)},
      {L"TrayHidden", JoinLines(settings.trayHidden)},
  };
}

Settings FromValues(const SettingValues& values) {
  Settings settings;
  ReadEnum(values, L"Edge", kEdges, settings.edge);
  ReadEnum(values, L"Theme", kThemes, settings.theme);
  ReadEnum(values, L"Language", kLanguages, settings.language);
  ReadBool(values, L"HideSystemTaskbar", settings.hideSystemTaskbar);
  ReadBool(values, L"WindowsKeyOpensMenu", settings.windowsKeyOpensMenu);
  ReadBool(values, L"StartWithWindows", settings.startWithWindows);
  ReadBool(values, L"WelcomeShown", settings.welcomeShown);
  ReadBool(values, L"ShowSearchButton", settings.showSearchButton);
  ReadBool(values, L"ShowTaskViewButton", settings.showTaskViewButton);
  ReadBool(values, L"TaskbarLocked", settings.taskbarLocked);
  if (const auto it = values.find(L"StartStyle"); it != values.end()) {
    if (const auto style = ParseStartStyle(it->second)) settings.startStyle = *style;
  }
  ReadEnum(values, L"MenuHorizontal", kHorizontals, settings.menuHorizontal);
  ReadEnum(values, L"MenuVertical", kVerticals, settings.menuVertical);
  ReadEnum(values, L"Orb", kOrbs, settings.orb);
  ReadRows(values, settings.menuRows);
  ReadBool(values, L"LargeIcons", settings.largeIcons);
  if (const auto it = values.find(L"PlacesShown"); it != values.end()) {
    for (auto& key : SplitLines(it->second)) SetPlaceShown(settings, key, true);
  }
  if (const auto it = values.find(L"PlacesHidden"); it != values.end()) {
    for (auto& key : SplitLines(it->second)) SetPlaceShown(settings, key, false);
  }
  if (const auto it = values.find(L"Pinned"); it != values.end()) {
    for (auto& target : SplitLines(it->second)) Pin(settings, target);
  }
  if (const auto it = values.find(L"TrayShown"); it != values.end()) {
    for (auto& identity : SplitLines(it->second)) SetTrayIconShown(settings, identity, true);
  }
  if (const auto it = values.find(L"TrayHidden"); it != values.end()) {
    for (auto& identity : SplitLines(it->second)) SetTrayIconShown(settings, identity, false);
  }
  return settings;
}

Settings WithOverrides(Settings settings, const Options& options) {
  if (options.edge) settings.edge = *options.edge;
  if (options.theme) settings.theme = *options.theme;
  if (options.language) settings.language = *options.language;
  if (options.startStyle) settings.startStyle = *options.startStyle;
  return settings;
}

void Pin(Settings& settings, const std::wstring& target) {
  if (!target.empty() && !IsPinned(settings, target)) settings.pinned.push_back(target);
}

void Unpin(Settings& settings, const std::wstring& target) { std::erase(settings.pinned, target); }

bool IsPinned(const Settings& settings, const std::wstring& target) {
  return std::ranges::find(settings.pinned, target) != settings.pinned.end();
}

bool IsPlaceShown(const Settings& settings, const std::wstring& key, bool shownByDefault) {
  return IsOverridden(settings.placesShown, settings.placesHidden, key, shownByDefault);
}

void SetPlaceShown(Settings& settings, const std::wstring& key, bool shown) {
  SetOverride(settings.placesShown, settings.placesHidden, key, shown);
}

bool IsTrayIconShown(const Settings& settings, const std::wstring& identity, bool windowsShows) {
  return IsOverridden(settings.trayShown, settings.trayHidden, identity, windowsShows);
}

void SetTrayIconShown(Settings& settings, const std::wstring& identity, bool shown) {
  SetOverride(settings.trayShown, settings.trayHidden, identity, shown);
}

}  // namespace throwback
