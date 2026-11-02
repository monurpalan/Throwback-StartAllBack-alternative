#pragma once

#include <span>
#include <string_view>

#include "ui/strings.h"

namespace throwback {

// A page of the Settings app the start menu search can open, like the
// Control Panel results of the Windows 7 search.
struct SettingsPage {
  ui::StringId name;
  std::wstring_view uri;
  // Extra words that find the page, in English and Turkish.
  std::wstring_view keywords;
};

std::span<const SettingsPage> SettingsPages();

// The Settings app, whose icon the results show.
inline constexpr std::wstring_view kSettingsApp =
    L"shell:AppsFolder\\windows.immersivecontrolpanel_cw5n1h2txyewy!microsoft.windows."
    L"immersivecontrolpanel";

}  // namespace throwback
