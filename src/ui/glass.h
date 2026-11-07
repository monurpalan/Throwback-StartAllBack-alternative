#pragma once

#include <windows.h>

namespace throwback::ui {

enum class GlassCorners { Square, Round, RoundSmall };

// Turns a popup window into a sheet of blurred glass, like Aero: whatever the
// canvas leaves transparent shows the blurred desktop behind the window.
void EnableGlass(HWND hwnd, GlassCorners corners);

// Lets the canvas's alpha show the desktop behind the window unblurred, for
// opaque looks with shaped edges like Windows XP's.
void EnableTransparency(HWND hwnd);

}  // namespace throwback::ui
