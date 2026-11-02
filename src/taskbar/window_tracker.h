#pragma once

#include <windows.h>

#include <string>
#include <vector>

#include "taskbar/app_icons.h"

namespace throwback {

struct TaskItem {
  HWND hwnd = nullptr;
  std::wstring title;
  HICON icon = nullptr;
  // False while the icon is a stand-in that should be looked up again.
  bool iconFinal = false;
  bool flashing = false;

  bool operator==(const TaskItem&) const = default;
};

// Keeps the list of windows that should have a taskbar button.
class WindowTracker {
 public:
  explicit WindowTracker(HWND self) : self_(self) {}

  // Re-reads the window list. Returns true when buttons, titles or icons changed.
  bool Refresh();
  // Makes the next Refresh() fetch the window's icon again.
  void InvalidateIcon(HWND hwnd);
  void SetFlashing(HWND hwnd, bool flashing);
  // The window whose button represents hwnd: itself, or the top-level owner
  // for dialogs and other windows without a button of their own.
  HWND ButtonOwner(HWND hwnd) const;

  const std::vector<TaskItem>& Items() const { return items_; }

 private:
  struct Icon {
    HICON handle = nullptr;
    bool final = false;
  };
  Icon ReadIcon(HWND hwnd);

  HWND self_;
  std::vector<TaskItem> items_;
  AppIconCache appIcons_;
};

}  // namespace throwback
