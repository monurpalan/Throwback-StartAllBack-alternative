#include "shell/settings_store.h"

#include <windows.h>

#include <utility>
#include <vector>

namespace throwback::shell {

SettingsStore::SettingsStore(std::wstring profile)
    : profile_(std::move(profile)), keyPath_(L"Software\\Throwback\\Profiles\\" + profile_) {}

Settings SettingsStore::Load() const {
  SettingValues values;
  HKEY key = nullptr;
  if (RegOpenKeyExW(HKEY_CURRENT_USER, keyPath_.c_str(), 0, KEY_READ, &key) != ERROR_SUCCESS) {
    return {};
  }
  for (const auto& [name, unused] : ToValues(Settings{})) {
    DWORD size = 0;
    if (RegGetValueW(key, nullptr, name.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &size) !=
        ERROR_SUCCESS) {
      continue;
    }
    std::vector<wchar_t> buffer(size / sizeof(wchar_t) + 1);
    if (RegGetValueW(key, nullptr, name.c_str(), RRF_RT_REG_SZ, nullptr, buffer.data(), &size) ==
        ERROR_SUCCESS) {
      values[name] = buffer.data();
    }
  }
  RegCloseKey(key);
  return FromValues(values);
}

void SettingsStore::Save(const Settings& settings) const {
  HKEY key = nullptr;
  if (RegCreateKeyExW(HKEY_CURRENT_USER, keyPath_.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &key,
                      nullptr) != ERROR_SUCCESS) {
    return;
  }
  for (const auto& [name, value] : ToValues(settings)) {
    RegSetValueExW(key, name.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()),
                   static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
  }
  RegCloseKey(key);
}

}  // namespace throwback::shell
