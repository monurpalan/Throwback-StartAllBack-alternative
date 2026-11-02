#include "core/settings.h"

#include <algorithm>
#include <array>
#include <string_view>
#include <utility>

namespace throwback {
namespace {

template <typename Enum, size_t N>
using Names = std::array<std::pair<Enum, std::wstring_view>, N>;

constexpr Names<Edge, 2> kEdges{{{Edge::Top, L"top"}, {Edge::Bottom, L"bottom"}}};
constexpr Names<ThemeMode, 3> kThemes{
    {{ThemeMode::System, L"system"}, {ThemeMode::Light, L"light"}, {ThemeMode::Dark, L"dark"}}};
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
      {L"Pinned", JoinLines(settings.pinned)},
  };
}

Settings FromValues(const SettingValues& values) {
  Settings settings;
  ReadEnum(values, L"Edge", kEdges, settings.edge);
  ReadEnum(values, L"Theme", kThemes, settings.theme);
  ReadEnum(values, L"Language", kLanguages, settings.language);
  ReadBool(values, L"HideSystemTaskbar", settings.hideSystemTaskbar);
  ReadBool(values, L"WindowsKeyOpensMenu", settings.windowsKeyOpensMenu);
  if (const auto it = values.find(L"Pinned"); it != values.end()) {
    for (auto& target : SplitLines(it->second)) Pin(settings, target);
  }
  return settings;
}

Settings WithOverrides(Settings settings, const Options& options) {
  if (options.edge) settings.edge = *options.edge;
  if (options.theme) settings.theme = *options.theme;
  if (options.language) settings.language = *options.language;
  return settings;
}

void Pin(Settings& settings, const std::wstring& target) {
  if (!target.empty() && !IsPinned(settings, target)) settings.pinned.push_back(target);
}

void Unpin(Settings& settings, const std::wstring& target) { std::erase(settings.pinned, target); }

bool IsPinned(const Settings& settings, const std::wstring& target) {
  return std::ranges::find(settings.pinned, target) != settings.pinned.end();
}

}  // namespace throwback
