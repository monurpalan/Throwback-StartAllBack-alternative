#pragma once

#include <windows.h>

#include <unordered_map>

#include "core/task_progress.h"

namespace throwback {

// What a window reported through ITaskbarList3 about itself.
struct TaskStatus {
  ProgressState progress = ProgressState::None;
  float fraction = 0.0f;
  HICON overlay = nullptr;  // owned by TaskStatusStore
};

// Collects progress and overlay icons from ITaskbarList3 messages, which the
// shell delivers to the window named by the "TaskbandHWND" property.
class TaskStatusStore {
 public:
  TaskStatusStore() = default;
  ~TaskStatusStore();
  TaskStatusStore(const TaskStatusStore&) = delete;
  TaskStatusStore& operator=(const TaskStatusStore&) = delete;

  // True when message was an ITaskbarList message; *changed tells whether a
  // button needs repainting.
  bool Handle(UINT message, WPARAM wParam, LPARAM lParam, bool* changed);
  const TaskStatus* Find(HWND window) const;
  // Drops what closed windows left behind.
  void Prune();

 private:
  std::unordered_map<HWND, TaskStatus> status_;
};

}  // namespace throwback
