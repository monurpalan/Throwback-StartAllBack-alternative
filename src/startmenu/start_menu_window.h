#pragma once

#include <windows.h>

#include <functional>
#include <memory>

#include "core/settings.h"

namespace throwback {

// Posted to the bar by a start menu item that opens Taskbar and Start Menu
// Properties.
constexpr UINT kOpenSettingsMessage = WM_APP + 4;

// Where a start menu opens: next to the start button (screen coordinates),
// below it for a bar at the top of the screen and above it otherwise, unless
// the user moved it to the right or to the other edge of the screen.
struct MenuAnchor {
  RECT button{};
  bool below = false;
  unsigned dpi = 96;
  MenuHorizontal horizontal = MenuHorizontal::Left;
  MenuVertical vertical = MenuVertical::Bar;
};

// The top-left corner of a width x height menu on the anchor's monitor,
// gap pixels off the bar or screen edge it touches.
POINT PlaceMenuWindow(const MenuAnchor& anchor, int width, int height, int gap = 0);

// A start menu of any style, as the bar sees it.
class StartMenuWindow {
 public:
  virtual ~StartMenuWindow() = default;
  virtual void Open(const MenuAnchor& anchor) = 0;
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
