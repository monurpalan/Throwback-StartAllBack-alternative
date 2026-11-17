#pragma once

#include <windows.h>

namespace throwback {

// The blinking caret of a start menu's search box, at the rate set in
// Windows. Typing shows it at once and starts its blink over, as in an edit
// box; it is drawn only while the menu has the keyboard.
class CaretBlink {
 public:
  static constexpr UINT_PTR kTimer = 0xCA7;

  void Start(HWND hwnd) {
    on_ = true;
    const UINT rate = GetCaretBlinkTime();
    // INFINITE means blinking is turned off: a steady caret.
    if (rate != 0 && rate != INFINITE) SetTimer(hwnd, kTimer, rate, nullptr);
  }
  void Stop(HWND hwnd) { KillTimer(hwnd, kTimer); }
  void Reset(HWND hwnd) {
    Start(hwnd);
    InvalidateRect(hwnd, nullptr, FALSE);
  }
  // Handles the blink timer; false for any other timer.
  bool OnTimer(HWND hwnd, WPARAM timer) {
    if (timer != kTimer) return false;
    on_ = !on_;
    InvalidateRect(hwnd, nullptr, FALSE);
    return true;
  }
  bool Visible(HWND hwnd) const { return on_ && GetFocus() == hwnd; }

 private:
  bool on_ = true;
};

}  // namespace throwback
