#include "core/win_key.h"

namespace throwback {
namespace {

// Virtual-key codes, spelled out to keep this file free of Win32 headers.
constexpr unsigned kLeftWin = 0x5B;
constexpr unsigned kRightWin = 0x5C;
constexpr unsigned kEscape = 0x1B;
constexpr unsigned kControl = 0x11;
constexpr unsigned kLeftControl = 0xA2;
constexpr unsigned kRightControl = 0xA3;

bool IsWin(unsigned key) { return key == kLeftWin || key == kRightWin; }
bool IsControl(unsigned key) {
  return key == kControl || key == kLeftControl || key == kRightControl;
}

}  // namespace

WinKeyAction WinKeyTracker::OnKey(unsigned virtualKey, bool down) {
  if (IsWin(virtualKey)) {
    if (down) {
      // Key repeat sends more downs; only the first one starts a press.
      if (!winDown_) comboUsed_ = false;
      winDown_ = true;
      return WinKeyAction::PassThrough;
    }
    const bool alone = winDown_ && !comboUsed_;
    winDown_ = false;
    return alone ? WinKeyAction::OpenMenuInsteadOfRelease : WinKeyAction::PassThrough;
  }

  if (IsControl(virtualKey)) {
    ctrlDown_ = down;
  } else if (down && virtualKey == kEscape && ctrlDown_ && !winDown_) {
    return WinKeyAction::OpenMenuAndSwallow;
  }
  if (down && winDown_) comboUsed_ = true;
  return WinKeyAction::PassThrough;
}

}  // namespace throwback
