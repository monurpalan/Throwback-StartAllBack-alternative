#pragma once

#include <windows.h>

#include <array>
#include <functional>

#include "core/settings.h"

namespace throwback {

// The start menu styles in the order lists show them, with their product
// names, the same in every language.
struct StyleName {
  StartStyle style;
  const wchar_t* name;
};
inline constexpr std::array<StyleName, 11> kStartStyles{{
    {StartStyle::Win98, L"Windows 98"},
    {StartStyle::WinMe, L"Windows Me"},
    {StartStyle::Win2000, L"Windows 2000"},
    {StartStyle::WinXp, L"Windows XP"},
    {StartStyle::Vista, L"Windows Vista"},
    {StartStyle::Win7, L"Windows 7"},
    {StartStyle::Plain8, L"Plain8 (Windows 7)"},
    {StartStyle::Win8, L"Windows 8"},
    {StartStyle::Win81, L"Windows 8.1"},
    {StartStyle::Win10, L"Windows 10"},
    {StartStyle::Win11, L"Windows 11"},
}};

// The "Taskbar and Start Menu Properties" window, built from native controls
// like the Windows 7 one. Shows modally over owner; apply receives the edited
// settings on OK and Apply. Showing it again while open brings it forward.
void ShowSettingsDialog(HINSTANCE instance, HWND owner, const Settings& current,
                        const std::function<void(const Settings&)>& apply);

}  // namespace throwback
