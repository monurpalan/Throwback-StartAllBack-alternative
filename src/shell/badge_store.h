#pragma once

#include <windows.h>

#include <map>
#include <optional>
#include <string>

#include "core/badge.h"

namespace throwback::shell {

// The badges apps have set, read from Windows' own notification database
// (%LOCALAPPDATA%\Microsoft\Windows\Notifications\wpndatabase.db), where
// Explorer's taskbar finds them too. Read-only, and only again after the
// database has changed.
class BadgeStore {
 public:
  // Re-reads when the database changed; true when a badge changed.
  bool Refresh();
  bool Empty() const { return badges_.empty(); }
  // The badge of an app, by its AppUserModelID (case-insensitive).
  std::optional<Badge> For(const std::wstring& appId) const;

 private:
  std::map<std::wstring, Badge> badges_;  // by lower-case app id
  ULONGLONG stamp_ = 0;                   // last write of the database files
};

}  // namespace throwback::shell
