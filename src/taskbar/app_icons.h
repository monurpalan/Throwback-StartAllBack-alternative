#pragma once

#include <windows.h>

#include "shell/shell_icons.h"

namespace throwback {

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
