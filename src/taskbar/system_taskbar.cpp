#include "taskbar/system_taskbar.h"

#include <shellapi.h>

#include <vector>

#include "taskbar/explorer_tray.h"

namespace throwback {
namespace {

constexpr wchar_t kRecoveryKey[] = L"Software\\Throwback";
constexpr wchar_t kRecoveryValue[] = L"SystemTaskbarState";

std::vector<HWND> ExplorerTaskbars() {
  std::vector<HWND> bars;
  if (HWND main = ExplorerTray()) bars.push_back(main);
  HWND secondary = nullptr;
  while ((secondary = FindWindowExW(nullptr, secondary, L"Shell_SecondaryTrayWnd", nullptr))) {
    bars.push_back(secondary);
  }
  return bars;
}

LPARAM GetState(HWND tray) {
  APPBARDATA data{sizeof(data)};
  data.hWnd = tray;
  return static_cast<LPARAM>(SHAppBarMessage(ABM_GETSTATE, &data));
}

void SetState(HWND tray, LPARAM state) {
  APPBARDATA data{sizeof(data)};
  data.hWnd = tray;
  data.lParam = state;
  SHAppBarMessage(ABM_SETSTATE, &data);
}

void ShowAll(bool show) {
  for (HWND bar : ExplorerTaskbars()) {
    if ((IsWindowVisible(bar) != FALSE) != show) ShowWindow(bar, show ? SW_SHOWNA : SW_HIDE);
  }
}

void WriteRecovery(LPARAM state) {
  const DWORD value = static_cast<DWORD>(state);
  RegSetKeyValueW(HKEY_CURRENT_USER, kRecoveryKey, kRecoveryValue, REG_DWORD, &value,
                  sizeof(value));
}

void ClearRecovery() { RegDeleteKeyValueW(HKEY_CURRENT_USER, kRecoveryKey, kRecoveryValue); }

void Restore(LPARAM state) {
  if (HWND tray = ExplorerTray()) SetState(tray, state);
  ShowAll(true);
}

}  // namespace

SystemTaskbar::SystemTaskbar() {
  HWND tray = ExplorerTray();
  if (!tray) return;  // no Explorer shell: nothing to hide
  originalState_ = GetState(tray);
  WriteRecovery(originalState_);
  applied_ = true;
  Enforce();
}

SystemTaskbar::~SystemTaskbar() {
  if (!applied_) return;
  Restore(originalState_);
  ClearRecovery();
}

void SystemTaskbar::Enforce() {
  if (!applied_) return;
  HWND tray = ExplorerTray();
  if (!tray) return;
  // Auto-hide makes Explorer release the screen edge it reserved.
  if ((GetState(tray) & ABS_AUTOHIDE) == 0) SetState(tray, originalState_ | ABS_AUTOHIDE);
  ShowAll(false);
}

void SystemTaskbar::RecoverFromCrash() {
  DWORD state = 0;
  DWORD size = sizeof(state);
  if (RegGetValueW(HKEY_CURRENT_USER, kRecoveryKey, kRecoveryValue, RRF_RT_REG_DWORD, nullptr,
                   &state, &size) != ERROR_SUCCESS) {
    return;
  }
  Restore(static_cast<LPARAM>(state));
  ClearRecovery();
}

}  // namespace throwback
