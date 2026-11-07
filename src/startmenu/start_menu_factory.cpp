#include <utility>

#include "startmenu/start_menu.h"
#include "startmenu/start_menu_window.h"

namespace throwback {

std::unique_ptr<StartMenuWindow> CreateStartMenu(HINSTANCE instance, HWND owner, Settings& settings,
                                                 std::function<void()> onClosed,
                                                 std::function<void()> onPinnedChanged) {
  return std::make_unique<StartMenu>(instance, owner, settings, std::move(onClosed),
                                     std::move(onPinnedChanged));
}

}  // namespace throwback
