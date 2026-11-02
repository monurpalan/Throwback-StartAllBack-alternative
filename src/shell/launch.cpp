#include "shell/launch.h"

#include <shellapi.h>

namespace throwback::shell {

bool Launch(const std::wstring& target, bool elevated) {
  SHELLEXECUTEINFOW info{};
  info.cbSize = sizeof(info);
  info.fMask = SEE_MASK_FLAG_LOG_USAGE | SEE_MASK_ASYNCOK;
  info.lpVerb = elevated ? L"runas" : nullptr;
  info.lpFile = target.c_str();
  info.nShow = SW_SHOWNORMAL;
  return ShellExecuteExW(&info) != FALSE;
}

void BringToFront(HWND window) {
  if (SetForegroundWindow(window)) return;
  // Only the process that received the last input may change the foreground
  // window. Pressing an unassigned key gives that right back to us.
  INPUT inputs[2] = {};
  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = 0xE8;
  inputs[1] = inputs[0];
  inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
  SendInput(2, inputs, sizeof(INPUT));
  SetForegroundWindow(window);
}

}  // namespace throwback::shell
