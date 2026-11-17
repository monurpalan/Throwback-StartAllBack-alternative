#pragma once

#include <windows.h>

#include <string>

#include "shell/shell_icons.h"

namespace throwback {

// The AppUserModelID of a window: its own property, or else its process's
// package identity (packaged apps such as WhatsApp). Empty for plain programs.
std::wstring AppIdOf(HWND hwnd);

// Icons of packaged (store) apps. Their frame windows don't carry the app's
// icon, so it is looked up through the AppUserModelID in shell:AppsFolder.
class AppIconCache {
 public:
  // Returns nullptr while the window doesn't expose an AppUserModelID yet,
  // which happens for a moment after a store app starts.
  HICON ForWindow(HWND hwnd);

 private:
  shell::IconCache icons_;
};

}  // namespace throwback
