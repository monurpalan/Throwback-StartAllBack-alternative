#include "taskbar/system_taskbar.h"

#include <shellapi.h>

#include <string>
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

void StartWatchdog() {
  static bool started = false;  // one per run is enough
  if (started) return;
  wchar_t path[MAX_PATH] = {};
  if (GetModuleFileNameW(nullptr, path, MAX_PATH) == 0) return;
  std::wstring command = L"\"" + std::wstring(path) + L"\" " + SystemTaskbar::kWatchdogArgument +
                         std::to_wstring(GetCurrentProcessId());
  STARTUPINFOW startup{sizeof(startup)};
  PROCESS_INFORMATION process{};
  if (CreateProcessW(path, command.data(), nullptr, nullptr, FALSE, DETACHED_PROCESS, nullptr,
                     nullptr, &startup, &process)) {
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    started = true;
  }
}

}  // namespace

SystemTaskbar::SystemTaskbar() {
  HWND tray = ExplorerTray();
  if (!tray) return;  // no Explorer shell: nothing to hide
  originalState_ = GetState(tray);
  WriteRecovery(originalState_);
  applied_ = true;
  StartWatchdog();
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

int SystemTaskbar::RunWatchdog(DWORD processId) {
  HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, processId);
  if (!process) return 0;
  WaitForSingleObject(process, INFINITE);
  CloseHandle(process);
  // A run that exited normally has restored the taskbar and cleared the
  // recovery value already; then this does nothing.
  RecoverFromCrash();
  return 0;
}

}  // namespace throwback
