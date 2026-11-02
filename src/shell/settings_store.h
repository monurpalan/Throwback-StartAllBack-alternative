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

 private:
  std::wstring keyPath_;
};

}  // namespace throwback::shell
