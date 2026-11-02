#pragma once

#include <windows.h>

#include <string>
#include <vector>

namespace throwback {

struct TaskItem {
  HWND hwnd = nullptr;
  std::wstring title;
  HICON icon = nullptr;

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

  const std::vector<TaskItem>& Items() const { return items_; }

 private:
  HWND self_;
  std::vector<TaskItem> items_;
};

}  // namespace throwback
