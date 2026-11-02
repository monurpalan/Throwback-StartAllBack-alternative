#pragma once

#include <array>

namespace throwback {

// The commands of a window's classic system menu, in menu order.
enum class WindowCommand { Restore, Move, Size, Minimize, Maximize, Close };

inline constexpr std::array<WindowCommand, 6> kWindowCommands = {
    WindowCommand::Restore,  WindowCommand::Move,     WindowCommand::Size,
    WindowCommand::Minimize, WindowCommand::Maximize, WindowCommand::Close,
};

struct WindowMenuState {
  bool minimized = false;
  bool maximized = false;
  bool canMinimize = false;  // WS_MINIMIZEBOX
  bool canMaximize = false;  // WS_MAXIMIZEBOX
  bool sizable = false;      // WS_THICKFRAME
};

// Same enabled rules as the system menu Windows builds for the window.
bool IsCommandEnabled(WindowCommand command, const WindowMenuState& state);
// The bold item: what a double click on the menu would do.
WindowCommand DefaultCommand(const WindowMenuState& state);

}  // namespace throwback
