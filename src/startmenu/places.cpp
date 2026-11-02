#include "startmenu/places.h"

#include <windows.h>
#include <knownfolders.h>
#include <sddl.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <wrl/client.h>

#include <filesystem>
#include <string_view>

#include "ui/strings.h"

#define SECURITY_WIN32
#include <security.h>

using Microsoft::WRL::ComPtr;

namespace throwback {
namespace {

std::wstring ItemName(IShellItem* item) {
  PWSTR name = nullptr;
  std::wstring result;
  if (SUCCEEDED(item->GetDisplayName(SIGDN_NORMALDISPLAY, &name))) result = name;
  CoTaskMemFree(name);
  return result;
}

// A known folder, opened by path so Explorer shows the real location.
void AddKnownFolder(std::vector<Place>& places, REFKNOWNFOLDERID id, bool separator = false) {
  ComPtr<IShellItem> item;
  if (FAILED(
          SHGetKnownFolderItem(id, KF_FLAG_DEFAULT, nullptr, IID_PPV_ARGS(item.GetAddressOf())))) {
    return;
  }
  PWSTR path = nullptr;
  if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
    places.push_back({ItemName(item.Get()), path, separator});
  }
  CoTaskMemFree(path);
}

// A virtual shell location such as "shell:::{CLSID}".
void AddShellPlace(std::vector<Place>& places, const wchar_t* target, bool separator = false,
                   std::wstring_view nameOverride = {}) {
  ComPtr<IShellItem> item;
  if (SUCCEEDED(SHCreateItemFromParsingName(target, nullptr, IID_PPV_ARGS(item.GetAddressOf())))) {
    const std::wstring name =
        nameOverride.empty() ? ItemName(item.Get()) : std::wstring(nameOverride);
    places.push_back({name, target, separator});
  }
}

std::wstring CurrentUserSid() {
  HANDLE token = nullptr;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) return {};
  BYTE buffer[256];
  DWORD size = 0;
  std::wstring result;
  if (GetTokenInformation(token, TokenUser, buffer, sizeof(buffer), &size)) {
    LPWSTR sid = nullptr;
    if (ConvertSidToStringSidW(reinterpret_cast<TOKEN_USER*>(buffer)->User.Sid, &sid)) {
      result = sid;
      LocalFree(sid);
    }
  }
  CloseHandle(token);
  return result;
}

std::wstring AccountPicturePath() {
  const std::wstring key =
      L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AccountPicture\\Users\\" + CurrentUserSid();
  wchar_t path[MAX_PATH] = {};
  DWORD size = sizeof(path);
  for (const wchar_t* value : {L"Image96", L"Image192", L"Image64"}) {
    size = sizeof(path);
    if (RegGetValueW(HKEY_LOCAL_MACHINE, key.c_str(), value, RRF_RT_REG_SZ, nullptr, path, &size) ==
        ERROR_SUCCESS) {
      std::error_code error;
      if (std::filesystem::exists(path, error)) return path;
    }
  }
  return {};
}

}  // namespace

std::vector<Place> LoadPlaces() {
  std::vector<Place> places;
  AddKnownFolder(places, FOLDERID_Profile);
  AddKnownFolder(places, FOLDERID_Documents);
  AddKnownFolder(places, FOLDERID_Pictures);
  AddKnownFolder(places, FOLDERID_Music);
  AddKnownFolder(places, FOLDERID_Downloads);
  // This PC, Control Panel, Devices and Printers, Run.
  AddShellPlace(places, L"shell:::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", /*separator=*/true);
  AddShellPlace(places, L"shell:::{26EE0668-A00A-44D7-9371-BEB064C98683}");
  AddShellPlace(places, L"shell:::{A8A91A66-3A7D-4424-8D24-04E180695C7A}");
  AddShellPlace(places, L"shell:::{2559A1F3-21D7-11D4-BDAF-00C04F60B9F0}", /*separator=*/true,
                ui::Tr(ui::StringId::Run));
  return places;
}

Account LoadAccount() {
  Account account;
  wchar_t name[256] = {};
  ULONG size = 256;
  if (GetUserNameExW(NameDisplay, name, &size) && name[0]) {
    account.name = name;
  } else {
    DWORD plainSize = 256;
    if (GetUserNameW(name, &plainSize)) account.name = name;
  }
  account.picturePath = AccountPicturePath();
  return account;
}

}  // namespace throwback
