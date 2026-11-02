#include <windows.h>

#include "ui/theme.h"

namespace throwback::ui {

bool SystemUsesLightTheme() {
  DWORD value = 0;
  DWORD size = sizeof(value);
  const LSTATUS status = RegGetValueW(
      HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
      L"SystemUsesLightTheme", RRF_RT_REG_DWORD, nullptr, &value, &size);
  // Windows 11 defaults the taskbar to dark when the value is missing.
  return status == ERROR_SUCCESS && value != 0;
}

}  // namespace throwback::ui
