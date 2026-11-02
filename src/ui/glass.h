#pragma once

#include <windows.h>

namespace throwback::ui {

enum class GlassCorners { Square, Round, RoundSmall };

// Turns a popup window into a sheet of blurred glass, like Aero: whatever the
// canvas leaves transparent shows the blurred desktop behind the window.
void EnableGlass(HWND hwnd, GlassCorners corners);

}  // namespace throwback::ui
