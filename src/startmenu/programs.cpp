#include "startmenu/programs.h"

#include <windows.h>
#include <knownfolders.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <wrl/client.h>

#include <algorithm>
#include <filesystem>
#include <unordered_set>

#include "core/search.h"

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

Program* FindChild(Program& parent, const std::wstring& name, bool folder) {
  const std::wstring key = FoldForSearch(name);
  for (auto& child : parent.children) {
    if (child->folder == folder && FoldForSearch(child->name) == key) return child.get();
  }
  return nullptr;
}

// Adds a Start Menu folder's contents under parent. Called for the user's and
// the shared folder in turn, so folders with the same name merge into one.
void AddFolder(Program& parent, const fs::path& directory) {
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
      AddFolder(*folder, path);
    } else if (!FindChild(parent, name, false)) {
      auto program = std::make_unique<Program>();
      program->name = name;
      program->target = path.wstring();
      parent.children.push_back(std::move(program));
    }
  }
}

// The classic order: programs first, then folders, each alphabetical. Empty
// folders are dropped.
void Tidy(Program& folder) {
  for (auto& child : folder.children) {
    if (child->folder) Tidy(*child);
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
  AddFolder(catalog->root, KnownFolderPath(FOLDERID_Programs));
  AddFolder(catalog->root, KnownFolderPath(FOLDERID_CommonPrograms));
  AddApps(*catalog);
  Tidy(catalog->root);
  std::unordered_set<std::wstring> seen;
  CollectPrograms(catalog->root, *catalog, seen);
  return catalog;
}

}  // namespace throwback
