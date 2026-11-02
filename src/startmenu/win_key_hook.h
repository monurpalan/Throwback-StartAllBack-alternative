#pragma once

#include <windows.h>

#include <thread>

namespace throwback {

// Catches a lone Windows key press (and Ctrl+Esc) system-wide and posts
// `message` to `target` instead of letting the system start menu open.
// Win+anything else is left untouched.
//
// Low-level hooks are dropped by Windows if they answer slowly, so the hook
// lives on its own thread that does nothing but pump messages.
class WinKeyHook {
 public:
  WinKeyHook(HWND target, UINT message);
  ~WinKeyHook();
  WinKeyHook(const WinKeyHook&) = delete;
  WinKeyHook& operator=(const WinKeyHook&) = delete;

 private:
  std::thread thread_;
  DWORD threadId_ = 0;
};

}  // namespace throwback
