#include "taskbar/task_status.h"

#include <iterator>

namespace throwback {
namespace {

// The messages ITaskbarList sends: wParam is the window, lParam the value.
constexpr UINT kFirstTaskbarListMessage = WM_USER + 50;
constexpr UINT kLastTaskbarListMessage = WM_USER + 87;
constexpr UINT kSetProgressValue = WM_USER + 64;
constexpr UINT kSetProgressState = WM_USER + 65;
constexpr UINT kSetOverlayIcon = WM_USER + 79;

void ReplaceOverlay(TaskStatus& status, HICON icon) {
  if (status.overlay) DestroyIcon(status.overlay);
  // The sender may destroy its icon right after the call.
  status.overlay = icon ? CopyIcon(icon) : nullptr;
}

}  // namespace

TaskStatusStore::~TaskStatusStore() {
  for (auto& [window, status] : status_) ReplaceOverlay(status, nullptr);
}

bool TaskStatusStore::Handle(UINT message, WPARAM wParam, LPARAM lParam, bool* changed) {
  if (message < kFirstTaskbarListMessage || message > kLastTaskbarListMessage) return false;
  *changed = false;
  const auto window = reinterpret_cast<HWND>(wParam);
  switch (message) {
    case kSetProgressValue:
      status_[window].fraction = ProgressFraction(static_cast<long long>(lParam));
      *changed = true;
      break;
    case kSetProgressState: {
      TaskStatus& status = status_[window];
      status.progress = ProgressStateFromFlags(static_cast<unsigned>(lParam));
      if (status.progress == ProgressState::None) status.fraction = 0.0f;
      *changed = true;
      break;
    }
    case kSetOverlayIcon:
      ReplaceOverlay(status_[window], reinterpret_cast<HICON>(lParam));
      *changed = true;
      break;
    default:
      break;  // tabs, thumbnail buttons and tooltips aren't shown
  }
  return true;
}

const TaskStatus* TaskStatusStore::Find(HWND window) const {
  const auto it = status_.find(window);
  return it == status_.end() ? nullptr : &it->second;
}

void TaskStatusStore::Prune() {
  for (auto it = status_.begin(); it != status_.end();) {
    if (IsWindow(it->first)) {
      ++it;
    } else {
      ReplaceOverlay(it->second, nullptr);
      it = status_.erase(it);
    }
  }
}

}  // namespace throwback
