#pragma once

#include <windows.h>

#include <string>

namespace throwback::shell {

// Opens a shell item the way Explorer would: programs, shortcuts, folders,
// "shell:AppsFolder\<app id>" and "ms-settings:" links. Launches are logged
// so Windows' own usage history (and our frequent list) stays accurate.
bool Launch(const std::wstring& target, bool elevated = false);

// Makes window the foreground window, even when Windows' focus-stealing
// rules would refuse because our process didn't receive the last input.
void BringToFront(HWND window);

}  // namespace throwback::shell
