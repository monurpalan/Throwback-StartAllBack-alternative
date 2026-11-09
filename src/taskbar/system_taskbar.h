#pragma once

#include <windows.h>

namespace throwback {

// Hides Explorer's taskbars (main and secondary monitors) while Throwback
// runs. They are switched to auto-hide, so they give back their screen space,
// and hidden; the original auto-hide state is restored on destruction.
//
// The original state is also written to the registry before anything
// changes, so a run that crashed is cleaned up by the next start.
class SystemTaskbar {
 public:
  SystemTaskbar();
  ~SystemTaskbar();
  SystemTaskbar(const SystemTaskbar&) = delete;
  SystemTaskbar& operator=(const SystemTaskbar&) = delete;

  // Explorer shows its taskbar again after some events and after restarting;
  // called periodically and on TaskbarCreated to keep it hidden.
  void Enforce();

  // Undoes what a crashed run left behind; safe to call when nothing is.
  static void RecoverFromCrash();

  // A run that is killed (Task Manager, a crash) cannot restore anything
  // itself, so hiding the taskbar also starts a watchdog: the same exe with
  // kWatchdogArgument, which waits for this process to end and then recovers.
  static constexpr wchar_t kWatchdogArgument[] = L"--restore-taskbar-after=";
  static int RunWatchdog(DWORD processId);

 private:
  bool applied_ = false;
  LPARAM originalState_ = 0;
};

}  // namespace throwback
