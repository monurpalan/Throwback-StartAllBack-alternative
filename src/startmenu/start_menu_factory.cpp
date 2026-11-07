#include <utility>

#include "startmenu/classic_start_menu.h"
#include "startmenu/start_menu.h"
#include "startmenu/start_menu_window.h"

namespace throwback {

std::unique_ptr<StartMenuWindow> CreateStartMenu(HINSTANCE instance, HWND owner, Settings& settings,
                                                 std::function<void()> onClosed,
                                                 std::function<void()> onPinnedChanged) {
  switch (settings.startStyle) {
    case StartStyle::Win98:
    case StartStyle::WinMe:
    case StartStyle::Win2000:
      return std::make_unique<ClassicStartMenu>(instance, owner, settings.startStyle,
                                                std::move(onClosed));
    default:
      break;
  }
  return std::make_unique<StartMenu>(instance, owner, settings, std::move(onClosed),
                                     std::move(onPinnedChanged));
}

}  // namespace throwback
