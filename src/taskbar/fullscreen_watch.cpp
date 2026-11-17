#include "taskbar/fullscreen_watch.h"

#include <string_view>
#include <utility>

#include "core/layout.h"

namespace throwback {

FullscreenWatch* FullscreenWatch::instance_ = nullptr;

FullscreenWatch::FullscreenWatch(std::function<void()> changed) : changed_(std::move(changed)) {
  instance_ = this;
  foregroundHook_ = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr,
                                    OnEvent, 0, 0, WINEVENT_OUTOFCONTEXT);
  Follow(GetForegroundWindow());
}

FullscreenWatch::~FullscreenWatch() {
  if (foregroundHook_) UnhookWinEvent(foregroundHook_);
  if (locationHook_) UnhookWinEvent(locationHook_);
  if (instance_ == this) instance_ = nullptr;
}

void FullscreenWatch::Follow(HWND foreground) {
  DWORD process = 0;
  const DWORD thread = foreground ? GetWindowThreadProcessId(foreground, &process) : 0;
  if (thread == followedThread_) return;
  if (locationHook_) UnhookWinEvent(locationHook_);
  locationHook_ = nullptr;
  followedThread_ = thread;
  if (thread == 0) return;
  locationHook_ = SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, nullptr,
                                  OnEvent, process, thread, WINEVENT_OUTOFCONTEXT);
}

void CALLBACK FullscreenWatch::OnEvent(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG object,
                                       LONG child, DWORD, DWORD) {
  FullscreenWatch* self = instance_;
  if (!self || object != OBJID_WINDOW || child != CHILDID_SELF) return;
  if (event == EVENT_SYSTEM_FOREGROUND) {
    self->Follow(hwnd);
  } else if (hwnd != GetForegroundWindow()) {
    return;  // another window of the same thread
  }
  self->changed_();
}

bool IsFullscreenWindow(HWND window, HMONITOR monitor) {
  if (!window || !IsWindowVisible(window) || IsIconic(window)) return false;
  if (MonitorFromWindow(window, MONITOR_DEFAULTTONULL) != monitor) return false;
  DWORD process = 0;
  GetWindowThreadProcessId(window, &process);
  if (process == GetCurrentProcessId()) return false;  // our own start screen
  wchar_t name[64] = {};
  GetClassNameW(window, name, 64);
  for (std::wstring_view shell :
       {L"Progman", L"WorkerW", L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd"}) {
    if (shell == name) return false;
  }
  MONITORINFO info{};
  info.cbSize = sizeof(info);
  RECT rect{};
  if (!GetMonitorInfoW(monitor, &info) || !GetWindowRect(window, &rect)) return false;
  const RECT& m = info.rcMonitor;
  return CoversMonitor({rect.left, rect.top, rect.right, rect.bottom},
                       {m.left, m.top, m.right, m.bottom});
}

}  // namespace throwback
