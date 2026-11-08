#pragma once

#include <string>

#include "core/settings.h"

namespace throwback::shell {

// Reads and writes one profile's settings under
// HKEY_CURRENT_USER\Software\Throwback\Profiles\<profile>.
class SettingsStore {
 public:
  explicit SettingsStore(std::wstring profile);

  Settings Load() const;
  void Save(const Settings& settings) const;
  const std::wstring& Profile() const { return profile_; }

 private:
  std::wstring profile_;
  std::wstring keyPath_;
};

}  // namespace throwback::shell
