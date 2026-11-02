#include "core/window_menu.h"

namespace throwback {

bool IsCommandEnabled(WindowCommand command, const WindowMenuState& state) {
  const bool normal = !state.minimized && !state.maximized;
  switch (command) {
    case WindowCommand::Restore:
      return !normal;
    case WindowCommand::Move:
      return normal;
    case WindowCommand::Size:
      return normal && state.sizable;
    case WindowCommand::Minimize:
      return state.canMinimize && !state.minimized;
    case WindowCommand::Maximize:
      return state.canMaximize && !state.maximized;
    case WindowCommand::Close:
      return true;
  }
  return false;
}

WindowCommand DefaultCommand(const WindowMenuState& state) {
  return state.minimized ? WindowCommand::Restore : WindowCommand::Close;
}

}  // namespace throwback
