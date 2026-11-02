#pragma once

#include <windows.h>

#include <string>
#include <unordered_map>

namespace throwback::shell {

// The icon of any shell item: a file path, "shell:AppsFolder\<app id>",
// "shell:::{CLSID}" and so on. The caller owns the returned icon.
HICON LoadItemIcon(const std::wstring& parsingName, int size);

// Owns icons keyed by parsing name and size; destroys them with the cache.
class IconCache {
 public:
  IconCache() = default;
  ~IconCache();
  IconCache(const IconCache&) = delete;
  IconCache& operator=(const IconCache&) = delete;

  // Loads on first use. Failures are remembered so they aren't retried.
  HICON Get(const std::wstring& parsingName, int size);

 private:
  std::unordered_map<std::wstring, HICON> icons_;
};

}  // namespace throwback::shell
