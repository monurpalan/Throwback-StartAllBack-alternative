#include "shell/recent_items.h"

#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <wrl/client.h>

// Needs the property types from the shell headers above.
#include <propkey.h>

#include <algorithm>
#include <filesystem>
#include <string_view>
#include <utility>

using Microsoft::WRL::ComPtr;

namespace throwback::shell {
namespace {

std::wstring DisplayName(IShellItem* item, SIGDN form) {
  PWSTR name = nullptr;
  std::wstring result;
  if (SUCCEEDED(item->GetDisplayName(form, &name))) result = name;
  CoTaskMemFree(name);
  return result;
}

// Jump list entries are shell items or, for some apps, shell links.
bool ReadEntry(IObjectArray* items, UINT index, RecentItem& out) {
  ComPtr<IShellItem> item;
  if (SUCCEEDED(items->GetAt(index, IID_PPV_ARGS(item.GetAddressOf())))) {
    out = {DisplayName(item.Get(), SIGDN_NORMALDISPLAY),
           DisplayName(item.Get(), SIGDN_DESKTOPABSOLUTEPARSING)};
    return !out.name.empty() && !out.target.empty();
  }
  ComPtr<IShellLinkW> link;
  if (SUCCEEDED(items->GetAt(index, IID_PPV_ARGS(link.GetAddressOf())))) {
    wchar_t path[MAX_PATH] = {};
    if (FAILED(link->GetPath(path, MAX_PATH, nullptr, 0)) || !path[0]) return false;
    wchar_t title[MAX_PATH] = {};
    link->GetDescription(title, MAX_PATH);
    out = {title[0] ? title : path, path};
    return true;
  }
  return false;
}

}  // namespace

std::wstring AppIdForTarget(const std::wstring& target) {
  constexpr std::wstring_view kAppsFolder = L"shell:AppsFolder\\";
  if (target.starts_with(kAppsFolder)) return target.substr(kAppsFolder.size());
  if (!target.ends_with(L".lnk")) return {};
  ComPtr<IPropertyStore> store;
  if (FAILED(SHGetPropertyStoreFromParsingName(target.c_str(), nullptr, GPS_DEFAULT,
                                               IID_PPV_ARGS(store.GetAddressOf())))) {
    return {};
  }
  PROPVARIANT value;
  PropVariantInit(&value);
  std::wstring appId;
  if (SUCCEEDED(store->GetValue(PKEY_AppUserModel_ID, &value)) && value.vt == VT_LPWSTR) {
    appId = value.pwszVal;
  }
  PropVariantClear(&value);
  return appId;
}

std::vector<RecentItem> LoadRecentDocuments(size_t limit) {
  std::vector<RecentItem> result;
  PWSTR folder = nullptr;
  if (FAILED(SHGetKnownFolderPath(FOLDERID_Recent, 0, nullptr, &folder))) return result;
  const std::filesystem::path recent = folder;
  CoTaskMemFree(folder);

  std::vector<std::pair<std::filesystem::file_time_type, std::filesystem::path>> links;
  std::error_code error;
  for (const auto& entry : std::filesystem::directory_iterator(recent, error)) {
    if (entry.path().extension() == L".lnk") {
      links.emplace_back(entry.last_write_time(error), entry.path());
    }
  }
  std::ranges::sort(links, [](const auto& a, const auto& b) { return a.first > b.first; });
  for (const auto& [time, path] : links) {
    if (result.size() >= limit) break;
    result.push_back({path.stem().wstring(), path.wstring()});
  }
  return result;
}

std::vector<RecentItem> LoadRecentItems(const std::wstring& appId, size_t limit) {
  std::vector<RecentItem> result;
  if (appId.empty()) return result;
  ComPtr<IApplicationDocumentLists> lists;
  if (FAILED(CoCreateInstance(CLSID_ApplicationDocumentLists, nullptr, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(lists.GetAddressOf()))) ||
      FAILED(lists->SetAppID(appId.c_str()))) {
    return result;
  }
  ComPtr<IObjectArray> items;
  if (FAILED(lists->GetList(ADLT_RECENT, static_cast<UINT>(limit),
                            IID_PPV_ARGS(items.GetAddressOf())))) {
    return result;
  }
  UINT count = 0;
  items->GetCount(&count);
  for (UINT i = 0; i < count && result.size() < limit; ++i) {
    RecentItem entry;
    if (ReadEntry(items.Get(), i, entry)) result.push_back(std::move(entry));
  }
  return result;
}

}  // namespace throwback::shell
