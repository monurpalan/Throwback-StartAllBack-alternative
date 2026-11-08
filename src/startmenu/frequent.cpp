#include "startmenu/frequent.h"

#include <windows.h>

#include <cstdint>
#include <span>
#include <string>

namespace throwback {
namespace {

// The UserAssist key that records programs by app ID.
constexpr wchar_t kAppIdUsage[] = L"{CEBFF5CD-ACE2-4F4F-9178-9926F41749EA}";

}  // namespace

std::vector<UsageEntry> ReadUserAssist(const wchar_t* guid) {
  std::vector<UsageEntry> entries;
  const std::wstring path =
      std::wstring(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist\\") + guid +
      L"\\Count";
  HKEY key = nullptr;
  if (RegOpenKeyExW(HKEY_CURRENT_USER, path.c_str(), 0, KEY_READ, &key) != ERROR_SUCCESS) {
    return entries;
  }
  wchar_t name[1024];
  std::uint8_t data[512];
  for (DWORD index = 0;; ++index) {
    DWORD nameLength = 1024;
    DWORD dataSize = sizeof(data);
    DWORD type = 0;
    const LSTATUS status =
        RegEnumValueW(key, index, name, &nameLength, nullptr, &type, data, &dataSize);
    if (status == ERROR_NO_MORE_ITEMS) break;
    if (status != ERROR_SUCCESS || type != REG_BINARY) continue;
    if (auto entry = ParseUsageValue({name, nameLength}, std::span(data, dataSize))) {
      entries.push_back(std::move(*entry));
    }
  }
  RegCloseKey(key);
  return entries;
}

std::vector<FrequentProgram> LoadFrequentPrograms(const ProgramCatalog& catalog, size_t limit) {
  std::vector<FrequentProgram> result;
  for (const UsageEntry& entry : RankFrequent(ReadUserAssist(kAppIdUsage), SIZE_MAX)) {
    const auto app = catalog.appNames.find(entry.appId);
    if (app == catalog.appNames.end()) continue;  // uninstalled, or not an app
    result.push_back({app->second, L"shell:AppsFolder\\" + entry.appId});
    if (result.size() == limit) break;
  }
  return result;
}

}  // namespace throwback
