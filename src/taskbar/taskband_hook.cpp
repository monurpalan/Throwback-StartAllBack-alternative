#include "taskbar/taskband_hook.h"

namespace throwback {
namespace {

constexpr wchar_t kProperty[] = L"TaskbandHWND";

}  // namespace

TaskbandHook::TaskbandHook(HWND receiver) : receiver_(receiver) { Reapply(); }

TaskbandHook::~TaskbandHook() { Restore(); }

void TaskbandHook::Reapply() {
  Restore();
  tray_ = FindWindowW(L"Shell_TrayWnd", nullptr);
  if (!tray_) return;
  previous_ = GetPropW(tray_, kProperty);
  SetPropW(tray_, kProperty, receiver_);
}

void TaskbandHook::Restore() {
  if (!tray_ || !IsWindow(tray_)) return;
  if (GetPropW(tray_, kProperty) == receiver_) {
    if (previous_) {
      SetPropW(tray_, kProperty, previous_);
    } else {
      RemovePropW(tray_, kProperty);
    }
  }
  tray_ = nullptr;
}

}  // namespace throwback
