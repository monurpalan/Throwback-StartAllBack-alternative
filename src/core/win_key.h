#pragma once

namespace throwback {

enum class WinKeyAction {
  PassThrough,
  // A Windows key was pressed and released alone: open our start menu, and
  // keep the system one closed by making the release look like a combination.
  OpenMenuInsteadOfRelease,
  // Ctrl+Esc: open our start menu and swallow the key.
  OpenMenuAndSwallow,
};

// Decides, key event by key event, when the user meant "open Start". Key
// events Throwback sends itself must not be fed in.
class WinKeyTracker {
 public:
  WinKeyAction OnKey(unsigned virtualKey, bool down);

 private:
  bool winDown_ = false;
  bool comboUsed_ = false;  // another key was pressed while Windows was held
  bool ctrlDown_ = false;
};

}  // namespace throwback
