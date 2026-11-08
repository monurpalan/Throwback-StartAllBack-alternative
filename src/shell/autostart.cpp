#include "shell/autostart.h"

#include <windows.h>
#include <shellapi.h>

#include <vector>

namespace throwback::shell {
namespace {

constexpr wchar_t kRunKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr wchar_t kDefaultProfile[] = L"default";

std::wstring RunValueName(const std::wstring& profile) {
  return profile == kDefaultProfile ? L"Throwback" : L"Throwback (" + profile + L")";
}

std::wstring ExecutablePath() {
  std::vector<wchar_t> path(MAX_PATH);
  for (;;) {
    const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    if (length == 0) return {};
    if (length < path.size()) return std::wstring(path.data(), length);
    path.resize(path.size() * 2);
  }
}

}  // namespace

void SetStartWithWindows(const std::wstring& profile, bool enabled) {
  HKEY key = nullptr;
  if (RegCreateKeyExW(HKEY_CURRENT_USER, kRunKey, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &key,
                      nullptr) != ERROR_SUCCESS) {
    return;
  }
  const std::wstring name = RunValueName(profile);
  const std::wstring path = ExecutablePath();
  if (enabled && !path.empty()) {
    std::wstring command = L"\"" + path + L"\"";
    if (profile != kDefaultProfile) command += L" --profile=" + profile;
    RegSetValueExW(key, name.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(command.c_str()),
                   static_cast<DWORD>((command.size() + 1) * sizeof(wchar_t)));
  } else {
    RegDeleteValueW(key, name.c_str());
  }
  RegCloseKey(key);
}

Edge SystemTaskbarEdge() {
  APPBARDATA data{};
  data.cbSize = sizeof(data);
  if (!SHAppBarMessage(ABM_GETTASKBARPOS, &data)) return Edge::Bottom;
  switch (data.uEdge) {
    case ABE_TOP:
      return Edge::Top;
    case ABE_LEFT:
      return Edge::Left;
    case ABE_RIGHT:
      return Edge::Right;
    default:
      return Edge::Bottom;
  }
}

}  // namespace throwback::shell
