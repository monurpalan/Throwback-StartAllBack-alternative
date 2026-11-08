#include "startmenu/places.h"

#include <windows.h>
#include <knownfolders.h>
#include <sddl.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <wrl/client.h>

#include <filesystem>
#include <string_view>
#include <utility>

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

// The Windows 7 places in menu order. A place names a known folder, a shell
// location (named by the shell unless it has a name of its own) or a link
// with a name of ours.
struct PlaceSpec {
  const wchar_t* key;
  const KNOWNFOLDERID* folder;
  const wchar_t* target;
  int name;  // a ui::StringId, or -1 for the shell's name
  int group;
  bool shownByDefault;
};

constexpr int kShellName = -1;
constexpr PlaceSpec kPlaces[] = {
    {L"profile", &FOLDERID_Profile, nullptr, kShellName, 0, true},
    {L"documents", &FOLDERID_Documents, nullptr, kShellName, 0, true},
    {L"pictures", &FOLDERID_Pictures, nullptr, kShellName, 0, true},
    {L"music", &FOLDERID_Music, nullptr, kShellName, 0, true},
    {L"videos", &FOLDERID_Videos, nullptr, kShellName, 0, false},
    {L"downloads", &FOLDERID_Downloads, nullptr, kShellName, 0, true},
    {L"recent", nullptr, L"shell:Recent", static_cast<int>(ui::StringId::VistaRecentItems), 0,
     false},
    {L"computer", nullptr, L"shell:::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", kShellName, 1, true},
    {L"network", nullptr, L"shell:::{F02C1A0D-BE21-4350-88B0-7367FC96EF3C}",
     static_cast<int>(ui::StringId::VistaNetwork), 1, false},
    {L"control-panel", nullptr, L"shell:::{26EE0668-A00A-44D7-9371-BEB064C98683}", kShellName, 1,
     true},
    {L"devices", nullptr, L"shell:::{A8A91A66-3A7D-4424-8D24-04E180695C7A}", kShellName, 1, true},
    {L"default-programs", nullptr, L"ms-settings:defaultapps",
     static_cast<int>(ui::StringId::VistaDefaultPrograms), 1, false},
    {L"help", nullptr, L"ms-contact-support:", static_cast<int>(ui::StringId::XpHelp), 1, false},
    {L"run", nullptr, L"shell:::{2559A1F3-21D7-11D4-BDAF-00C04F60B9F0}",
     static_cast<int>(ui::StringId::Run), 2, true},
};

// The place a spec stands for, or false when Windows doesn't have it.
bool ResolvePlace(const PlaceSpec& spec, Place& place) {
  if (spec.folder) {
    std::vector<Place> found;
    AddKnownFolder(found, *spec.folder);
    if (found.empty()) return false;
    place = found.front();
    return true;
  }
  if (spec.name != kShellName) {
    place = {std::wstring(ui::Tr(static_cast<ui::StringId>(spec.name))), spec.target};
    return true;
  }
  std::vector<Place> found;
  AddShellPlace(found, spec.target);
  if (found.empty()) return false;
  place = found.front();
  return true;
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

template <size_t N>
std::vector<Place> PlacesFrom(const PlaceSpec (&table)[N], const Settings& settings) {
  std::vector<Place> places;
  int lastGroup = -1;
  for (const PlaceSpec& spec : table) {
    Place place;
    if (!IsPlaceShown(settings, spec.key, spec.shownByDefault) || !ResolvePlace(spec, place)) {
      continue;
    }
    place.separatorBefore = lastGroup >= 0 && spec.group != lastGroup;
    lastGroup = spec.group;
    places.push_back(std::move(place));
  }
  return places;
}

template <size_t N>
std::vector<PlaceChoice> ChoicesFrom(const PlaceSpec (&table)[N], const Settings& settings) {
  std::vector<PlaceChoice> choices;
  for (const PlaceSpec& spec : table) {
    Place place;
    if (!ResolvePlace(spec, place)) continue;
    choices.push_back(
        {spec.key, place.name, IsPlaceShown(settings, spec.key, spec.shownByDefault)});
  }
  return choices;
}

}  // namespace

std::vector<Place> LoadPlaces(const Settings& settings) { return PlacesFrom(kPlaces, settings); }

namespace {

// Windows XP's right column, with its shell32 icons and its bold user places.
struct XpItem {
  const wchar_t* key;
  ui::StringId name;
  const wchar_t* target;
  int icon;
  bool bold;
  bool separatorBefore;
};
constexpr XpItem kXpItems[] = {
    {L"documents", ui::StringId::XpMyDocuments, L"shell:Personal", 235, true, false},
    {L"recent", ui::StringId::XpMyRecentDocuments, L"shell:Recent", 21, true, false},
    {L"pictures", ui::StringId::XpMyPictures, L"shell:My Pictures", 236, true, false},
    {L"music", ui::StringId::XpMyMusic, L"shell:My Music", 237, true, false},
    {L"computer", ui::StringId::XpMyComputer, L"shell:::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", 16,
     true, false},
    {L"control-panel", ui::StringId::ClassicControlPanel,
     L"shell:::{26EE0668-A00A-44D7-9371-BEB064C98683}", 137, false, true},
    {L"devices", ui::StringId::XpPrinters, L"shell:::{2227A280-3AEA-1069-A2DE-08002B30309D}", 138,
     false, false},
    {L"help", ui::StringId::XpHelp, L"ms-contact-support:", 24, false, true},
    {L"search", ui::StringId::ClassicSearch, L"search-ms:", 23, false, false},
    {L"run", ui::StringId::Run, L"shell:::{2559A1F3-21D7-11D4-BDAF-00C04F60B9F0}", 25, false,
     false},
};

// Windows Vista's right column, all shown until the user turns them off.
constexpr PlaceSpec kVistaPlaces[] = {
    {L"profile", &FOLDERID_Profile, nullptr, kShellName, 0, true},
    {L"documents", &FOLDERID_Documents, nullptr, kShellName, 0, true},
    {L"pictures", &FOLDERID_Pictures, nullptr, kShellName, 0, true},
    {L"music", &FOLDERID_Music, nullptr, kShellName, 0, true},
    {L"search", nullptr, L"search-ms:", static_cast<int>(ui::StringId::ClassicSearch), 1, true},
    {L"recent", nullptr, L"shell:Recent", static_cast<int>(ui::StringId::VistaRecentItems), 1,
     true},
    {L"computer", nullptr, L"shell:::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", kShellName, 1, true},
    {L"network", nullptr, L"shell:::{F02C1A0D-BE21-4350-88B0-7367FC96EF3C}",
     static_cast<int>(ui::StringId::VistaNetwork), 1, true},
    {L"control-panel", nullptr, L"shell:::{26EE0668-A00A-44D7-9371-BEB064C98683}", kShellName, 2,
     true},
    {L"default-programs", nullptr, L"ms-settings:defaultapps",
     static_cast<int>(ui::StringId::VistaDefaultPrograms), 2, true},
    {L"help", nullptr, L"ms-contact-support:", static_cast<int>(ui::StringId::XpHelp), 2, true},
};

}  // namespace

std::vector<PlaceChoice> LoadPlaceChoices(const Settings& settings) {
  if (settings.startStyle == StartStyle::WinXp) {
    std::vector<PlaceChoice> choices;
    for (const XpItem& item : kXpItems) {
      choices.push_back(
          {item.key, std::wstring(ui::Tr(item.name)), IsPlaceShown(settings, item.key, true)});
    }
    return choices;
  }
  if (settings.startStyle == StartStyle::Vista) return ChoicesFrom(kVistaPlaces, settings);
  return ChoicesFrom(kPlaces, settings);
}

std::vector<Place> LoadVistaPlaces(const Settings& settings) {
  return PlacesFrom(kVistaPlaces, settings);
}

std::vector<Place> LoadXpPlaces(const Settings& settings) {
  std::vector<Place> places;
  for (const XpItem& item : kXpItems) {
    if (!IsPlaceShown(settings, item.key, true)) continue;
    places.push_back(
        {std::wstring(ui::Tr(item.name)), item.target, item.separatorBefore, item.icon, item.bold});
  }
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
