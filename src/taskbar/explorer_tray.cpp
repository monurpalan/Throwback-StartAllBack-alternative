#include "taskbar/explorer_tray.h"

namespace throwback {

HWND ExplorerTray() {
  HWND candidate = nullptr;
  while ((candidate = FindWindowExW(nullptr, candidate, L"Shell_TrayWnd", nullptr))) {
    DWORD process = 0;
    GetWindowThreadProcessId(candidate, &process);
    if (process != GetCurrentProcessId()) return candidate;
  }
  return nullptr;
}

}  // namespace throwback
