#include "startmenu/power.h"

#include <windows.h>
#include <powrprof.h>

namespace throwback {
namespace {

// Shutting down and restarting need the shutdown privilege, which every
// interactive user has but no process has enabled by default.
void EnableShutdownPrivilege() {
  HANDLE token = nullptr;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
    return;
  }
  TOKEN_PRIVILEGES privileges{};
  privileges.PrivilegeCount = 1;
  privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  if (LookupPrivilegeValueW(nullptr, SE_SHUTDOWN_NAME, &privileges.Privileges[0].Luid)) {
    AdjustTokenPrivileges(token, FALSE, &privileges, 0, nullptr, nullptr);
  }
  CloseHandle(token);
}

constexpr DWORD kPlannedReason =
    SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED;

}  // namespace

void RunPowerAction(PowerAction action) {
  switch (action) {
    case PowerAction::ShutDown:
      EnableShutdownPrivilege();
      // Hybrid shutdown is what the system start menu does when fast startup is on.
      ExitWindowsEx(EWX_SHUTDOWN | EWX_POWEROFF | EWX_HYBRID_SHUTDOWN, kPlannedReason);
      break;
    case PowerAction::Restart:
      EnableShutdownPrivilege();
      ExitWindowsEx(EWX_REBOOT, kPlannedReason);
      break;
    case PowerAction::Sleep:
      SetSuspendState(FALSE, FALSE, FALSE);
      break;
    case PowerAction::SignOut:
      ExitWindowsEx(EWX_LOGOFF, 0);
      break;
    case PowerAction::Lock:
      LockWorkStation();
      break;
  }
}

}  // namespace throwback
