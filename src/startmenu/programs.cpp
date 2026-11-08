#include "startmenu/programs.h"

#include <windows.h>
#include <knownfolders.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <wrl/client.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "core/search.h"
#include "core/user_assist.h"
#include "startmenu/frequent.h"

using Microsoft::WRL::ComPtr;
namespace fs = std::filesystem;

namespace throwback {
namespace {

fs::path KnownFolderPath(REFKNOWNFOLDERID id) {
  PWSTR raw = nullptr;
  fs::path path;
  if (SUCCEEDED(SHGetKnownFolderPath(id, KF_FLAG_DEFAULT, nullptr, &raw))) path = raw;
  CoTaskMemFree(raw);
  return path;
}

// The name Explorer shows: localized through desktop.ini, without ".lnk".
std::wstring DisplayName(const fs::path& path) {
  SHFILEINFOW info{};
  if (SHGetFileInfoW(path.c_str(), 0, &info, sizeof(info), SHGFI_DISPLAYNAME)) {
    return info.szDisplayName;
  }
  return path.stem().wstring();
}

bool IsHidden(const fs::path& path) {
  const DWORD attributes = GetFileAttributesW(path.c_str());
  return attributes != INVALID_FILE_ATTRIBUTES &&
         (attributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) != 0;
}

// What marks newly installed shortcuts: when each Start Menu shortcut was
// last started, keyed like UserAssist names them ("{folder GUID}\\path" in
// lower case), and the time now.
struct FreshCheck {
  std::unordered_map<std::wstring, std::uint64_t> lastRun;
  std::uint64_t now = 0;
};

// The UserAssist key that records started shortcuts, and the known folder
// GUIDs it names the two Start Menu program folders by.
constexpr wchar_t kShortcutUsage[] = L"{F4E57C4B-2036-45F0-A9AB-443BCFE33D9F}";
constexpr wchar_t kUserPrograms[] = L"{A77F5D77-2E2B-44C3-A6A2-ABA601054A51}";
constexpr wchar_t kCommonPrograms[] = L"{0139D44E-6AFE-49F2-8690-3DAFCAE6FFB8}";

std::wstring Lower(std::wstring text) {
  CharLowerBuffW(text.data(), static_cast<DWORD>(text.size()));
  return text;
}

FreshCheck LoadFreshCheck() {
  FreshCheck check;
  for (const UsageEntry& entry : ReadUserAssist(kShortcutUsage)) {
    check.lastRun[Lower(entry.appId)] = entry.lastRun;
  }
  FILETIME now{};
  GetSystemTimeAsFileTime(&now);
  check.now = (static_cast<std::uint64_t>(now.dwHighDateTime) << 32) | now.dwLowDateTime;
  return check;
}

// Whether the shortcut at path (key as UserAssist would name it) is new.
bool IsFresh(const fs::path& path, const std::wstring& key, const FreshCheck& check) {
  WIN32_FILE_ATTRIBUTE_DATA data{};
  if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &data)) return false;
  const std::uint64_t created =
      (static_cast<std::uint64_t>(data.ftCreationTime.dwHighDateTime) << 32) |
      data.ftCreationTime.dwLowDateTime;
  const auto run = check.lastRun.find(Lower(key));
  return IsNewlyInstalled(created, check.now, run == check.lastRun.end() ? 0 : run->second);
}

Program* FindChild(Program& parent, const std::wstring& name, bool folder) {
  const std::wstring key = FoldForSearch(name);
  for (auto& child : parent.children) {
    if (child->folder == folder && FoldForSearch(child->name) == key) return child.get();
  }
  return nullptr;
}

// Adds a Start Menu folder's contents under parent. Called for the user's and
// the shared folder in turn, so folders with the same name merge into one.
// key is the folder as UserAssist names it.
void AddFolder(Program& parent, const fs::path& directory, const std::wstring& key,
               const FreshCheck& check) {
  std::error_code error;
  for (const auto& entry : fs::directory_iterator(directory, error)) {
    const fs::path& path = entry.path();
    if (IsHidden(path)) continue;
    const std::wstring name = DisplayName(path);
    if (entry.is_directory(error)) {
      Program* folder = FindChild(parent, name, true);
      if (!folder) {
        auto created = std::make_unique<Program>();
        created->name = name;
        created->target = path.wstring();  // only for the folder icon
        created->folder = true;
        folder = parent.children.emplace_back(std::move(created)).get();
      }
      AddFolder(*folder, path, key + L"\\" + path.filename().wstring(), check);
    } else if (!FindChild(parent, name, false)) {
      auto program = std::make_unique<Program>();
      program->name = name;
      program->target = path.wstring();
      program->fresh = IsFresh(path, key + L"\\" + path.filename().wstring(), check);
      parent.children.push_back(std::move(program));
    }
  }
}

// The classic order: programs first, then folders, each alphabetical. Empty
// folders are dropped.
void Tidy(Program& folder) {
  for (auto& child : folder.children) {
    if (child->folder) Tidy(*child);
    // A folder holding a new program is highlighted too.
    if (child->fresh) folder.fresh = true;
  }
  std::erase_if(folder.children,
                [](const auto& child) { return child->folder && child->children.empty(); });
  std::sort(folder.children.begin(), folder.children.end(), [](const auto& a, const auto& b) {
    if (a->folder != b->folder) return !a->folder;
    return CompareStringEx(LOCALE_NAME_USER_DEFAULT, LINGUISTIC_IGNORECASE, a->name.c_str(), -1,
                           b->name.c_str(), -1, nullptr, nullptr, 0) == CSTR_LESS_THAN;
  });
}

// Walks shell:AppsFolder: records every app ID's name, and adds packaged
// apps (which have no shortcut in the Start Menu folders) to the top level.
void AddApps(ProgramCatalog& catalog) {
  ComPtr<IShellItem> appsFolder;
  ComPtr<IEnumShellItems> items;
  if (FAILED(SHGetKnownFolderItem(FOLDERID_AppsFolder, KF_FLAG_DEFAULT, nullptr,
                                  IID_PPV_ARGS(appsFolder.GetAddressOf()))) ||
      FAILED(
          appsFolder->BindToHandler(nullptr, BHID_EnumItems, IID_PPV_ARGS(items.GetAddressOf())))) {
    return;
  }
  ComPtr<IShellItem> item;
  while (items->Next(1, item.ReleaseAndGetAddressOf(), nullptr) == S_OK) {
    PWSTR name = nullptr;
    PWSTR appId = nullptr;
    if (SUCCEEDED(item->GetDisplayName(SIGDN_NORMALDISPLAY, &name)) &&
        SUCCEEDED(item->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &appId))) {
      catalog.appNames.emplace(appId, name);
      // Packaged app IDs look like "Package_publisher!App".
      const bool packaged = std::wstring_view(appId).find(L'!') != std::wstring_view::npos;
      if (packaged && !FindChild(catalog.root, name, false)) {
        auto program = std::make_unique<Program>();
        program->name = name;
        program->target = std::wstring(L"shell:AppsFolder\\") + appId;
        catalog.root.children.push_back(std::move(program));
      }
    }
    CoTaskMemFree(name);
    CoTaskMemFree(appId);
  }
}

void CollectPrograms(const Program& folder, ProgramCatalog& catalog,
                     std::unordered_set<std::wstring>& seen) {
  for (const auto& child : folder.children) {
    if (child->folder) {
      CollectPrograms(*child, catalog, seen);
    } else if (seen.insert(FoldForSearch(child->name)).second) {
      catalog.all.push_back(child.get());
    }
  }
}

}  // namespace

std::unique_ptr<ProgramCatalog> LoadPrograms() {
  auto catalog = std::make_unique<ProgramCatalog>();
  catalog->root.folder = true;
  const FreshCheck check = LoadFreshCheck();
  AddFolder(catalog->root, KnownFolderPath(FOLDERID_Programs), kUserPrograms, check);
  AddFolder(catalog->root, KnownFolderPath(FOLDERID_CommonPrograms), kCommonPrograms, check);
  AddApps(*catalog);
  Tidy(catalog->root);
  catalog->hasFresh = catalog->root.fresh;
  std::unordered_set<std::wstring> seen;
  CollectPrograms(catalog->root, *catalog, seen);
  return catalog;
}

}  // namespace throwback
