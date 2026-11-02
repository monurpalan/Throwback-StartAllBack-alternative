#pragma once

#include <windows.h>

namespace throwback {

// Points Explorer's "TaskbandHWND" property at our window, so the progress
// and overlay icons apps set through ITaskbarList3 come to us. The old value
// is put back on destruction.
class TaskbandHook {
 public:
  explicit TaskbandHook(HWND receiver);
  ~TaskbandHook();
  TaskbandHook(const TaskbandHook&) = delete;
  TaskbandHook& operator=(const TaskbandHook&) = delete;

  // Explorer restarted: its new taskbar needs the property again.
  void Reapply();

 private:
  void Restore();

  HWND receiver_;
  HWND tray_ = nullptr;
  HANDLE previous_ = nullptr;
};

}  // namespace throwback
