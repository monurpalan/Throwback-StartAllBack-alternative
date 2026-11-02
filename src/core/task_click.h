#pragma once

namespace throwback {

enum class TaskClickAction { Activate, Restore, Minimize, None };

// The window's state when the mouse button went down. Captured at press
// time because the foreground window can change before the button is released.
struct TaskClickState {
  bool wasActive = false;
  bool minimized = false;
  bool canMinimize = false;  // has WS_MINIMIZEBOX and no modal dialog open
};

// Classic behaviour: clicking the active window's button minimizes it,
// clicking any other button brings that window to the front.
TaskClickAction DecideTaskClick(const TaskClickState& state);

}  // namespace throwback
