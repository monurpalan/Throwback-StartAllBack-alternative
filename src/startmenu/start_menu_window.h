#pragma once

#include <windows.h>

#include <functional>
#include <memory>

#include "core/settings.h"

namespace throwback {

// Posted to the bar by a start menu item that opens Taskbar and Start Menu
// Properties.
constexpr UINT kOpenSettingsMessage = WM_APP + 4;

// A start menu of any style, as the bar sees it.
class StartMenuWindow {
 public:
  virtual ~StartMenuWindow() = default;
  // anchor is the start button in screen coordinates; the menu opens below
  // it for a bar at the top of the screen and above it otherwise.
  virtual void Open(const RECT& anchor, bool below, unsigned dpi) = 0;
  // Dismisses the menu and gives focus back to the window that had it.
  virtual void Close() = 0;
  virtual bool IsOpen() const = 0;
};

// The menu is owned by the bar, so closing requests reach the bar. onClosed
// runs whenever the menu opens or closes; pinning changes settings.pinned
// and then calls onPinnedChanged to save.
std::unique_ptr<StartMenuWindow> CreateStartMenu(HINSTANCE instance, HWND owner, Settings& settings,
                                                 std::function<void()> onClosed,
                                                 std::function<void()> onPinnedChanged);

}  // namespace throwback
