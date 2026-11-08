#include <utility>

#include "core/start_menu_layout.h"

#include "startmenu/classic_start_menu.h"
#include "startmenu/modern_start_menu.h"
#include "startmenu/start_menu.h"
#include "startmenu/start_menu_window.h"

namespace throwback {

POINT PlaceMenuWindow(const MenuAnchor& anchor, int width, int height, int gap) {
  MONITORINFO monitor{};
  monitor.cbSize = sizeof(monitor);
  GetMonitorInfoW(MonitorFromRect(&anchor.button, MONITOR_DEFAULTTONEAREST), &monitor);
  const auto rect = [](const RECT& r) {
    return Rect{static_cast<int>(r.left), static_cast<int>(r.top), static_cast<int>(r.right),
                static_cast<int>(r.bottom)};
  };
  const MenuPosition position =
      PlaceStartMenu(rect(anchor.button), anchor.below, width, height, rect(monitor.rcMonitor),
                     rect(monitor.rcWork), anchor.horizontal, anchor.vertical, gap);
  return {position.x, position.y};
}

std::unique_ptr<StartMenuWindow> CreateStartMenu(HINSTANCE instance, HWND owner, Settings& settings,
                                                 std::function<void()> onClosed,
                                                 std::function<void()> onPinnedChanged) {
  switch (settings.startStyle) {
    case StartStyle::Win98:
    case StartStyle::WinMe:
    case StartStyle::Win2000:
      return std::make_unique<ClassicStartMenu>(instance, owner, settings.startStyle,
                                                std::move(onClosed));
    case StartStyle::Win8:
    case StartStyle::Win81:
    case StartStyle::Win10:
    case StartStyle::Win11:
      return std::make_unique<ModernStartMenu>(instance, owner, settings, std::move(onClosed));
    default:
      break;
  }
  return std::make_unique<StartMenu>(instance, owner, settings, settings.startStyle,
                                     std::move(onClosed), std::move(onPinnedChanged));
}

}  // namespace throwback
