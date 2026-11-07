#pragma once

#include <windows.h>

#include <array>
#include <cstdint>
#include <string>
#include <unordered_set>

namespace throwback {

// A notification icon's identity that survives restarts: its GUID when it has
// one, otherwise its program's path (lowercase) and its ID, or its window's
// class when the path can't be read.
std::wstring IconIdentity(HWND window, std::uint32_t id, const std::array<std::uint8_t, 16>& guid,
                          bool useGuid);

// Windows 11 remembers for each notification icon whether it sits on the
// taskbar or behind the arrow (HKCU\Control Panel\NotifyIconSettings);
// Throwback starts from that choice.
class WindowsIconChoices {
 public:
  void Load();
  bool Shows(const std::wstring& identity) const { return shown_.contains(identity); }

 private:
  std::unordered_set<std::wstring> shown_;
};

}  // namespace throwback
