#pragma once

#include <windows.h>

#include "core/settings.h"

namespace throwback {

// The "Customize Start Menu" window of the properties dialog: how many
// programs the Windows 7 style menus list, their icon size and which places
// the right column shows. Shows modally over owner and edits settings in
// place when the user presses OK; returns whether they did.
bool ShowCustomizeStartMenu(HINSTANCE instance, HWND owner, Settings& settings);

}  // namespace throwback
