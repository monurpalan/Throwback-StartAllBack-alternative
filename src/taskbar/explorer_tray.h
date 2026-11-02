#pragma once

#include <windows.h>

namespace throwback {

// Explorer's own taskbar window. While Throwback hosts the notification area
// it has a Shell_TrayWnd of its own, so plain FindWindow may return ours.
HWND ExplorerTray();

}  // namespace throwback
