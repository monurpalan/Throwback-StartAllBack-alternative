#pragma once

#include <windows.h>

#include <string>
#include <unordered_map>

namespace throwback {

// Icons of packaged (store) apps. Their frame windows don't carry the app's
// icon, so it is looked up through the AppUserModelID in shell:AppsFolder.
// Icons are created once per app and owned by the cache.
class AppIconCache {
 public:
  AppIconCache() = default;
  ~AppIconCache();
  AppIconCache(const AppIconCache&) = delete;
  AppIconCache& operator=(const AppIconCache&) = delete;

  // Returns nullptr while the window doesn't expose an AppUserModelID yet,
  // which happens for a moment after a store app starts.
  HICON ForWindow(HWND hwnd);

 private:
  std::unordered_map<std::wstring, HICON> icons_;
};

}  // namespace throwback
