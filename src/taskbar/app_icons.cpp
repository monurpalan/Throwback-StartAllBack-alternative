#include "taskbar/app_icons.h"

#include <knownfolders.h>
#include <propsys.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <wrl/client.h>

#include <vector>

using Microsoft::WRL::ComPtr;

namespace throwback {
namespace {

constexpr int kIconSize = 32;

// PKEY_AppUserModel_ID, spelled out to avoid pulling in the property key library.
constexpr PROPERTYKEY kAppUserModelId = {
    {0x9F4C2855, 0x9F79, 0x4B39, {0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3}}, 5};

std::wstring ReadAppUserModelId(HWND hwnd) {
  ComPtr<IPropertyStore> store;
  if (FAILED(SHGetPropertyStoreForWindow(hwnd, IID_PPV_ARGS(store.GetAddressOf())))) return {};

  PROPVARIANT value;
  PropVariantInit(&value);
  std::wstring id;
  if (SUCCEEDED(store->GetValue(kAppUserModelId, &value)) && value.vt == VT_LPWSTR &&
      value.pwszVal) {
    id = value.pwszVal;
  }
  PropVariantClear(&value);
  return id;
}

// Turns a 32-bit bitmap with alpha into an icon; the caller owns both.
HICON IconFromBitmap(HBITMAP color) {
  BITMAP info{};
  if (!GetObjectW(color, sizeof(info), &info)) return nullptr;
  // Monochrome masks are word aligned. An all-zero mask lets the alpha channel decide.
  const size_t stride = static_cast<size_t>((info.bmWidth + 15) / 16) * 2;
  const std::vector<BYTE> bits(stride * static_cast<size_t>(info.bmHeight), 0);
  HBITMAP mask = CreateBitmap(info.bmWidth, info.bmHeight, 1, 1, bits.data());

  ICONINFO iconInfo{TRUE, 0, 0, mask, color};
  HICON icon = CreateIconIndirect(&iconInfo);
  DeleteObject(mask);
  return icon;
}

HICON LoadAppIcon(const std::wstring& appId) {
  ComPtr<IShellItemImageFactory> factory;
  if (FAILED(SHCreateItemInKnownFolder(FOLDERID_AppsFolder, KF_FLAG_DEFAULT, appId.c_str(),
                                       IID_PPV_ARGS(factory.GetAddressOf())))) {
    return nullptr;
  }
  HBITMAP bitmap = nullptr;
  if (FAILED(factory->GetImage({kIconSize, kIconSize}, SIIGBF_ICONONLY, &bitmap))) return nullptr;
  HICON icon = IconFromBitmap(bitmap);
  DeleteObject(bitmap);
  return icon;
}

}  // namespace

AppIconCache::~AppIconCache() {
  for (const auto& [id, icon] : icons_) {
    if (icon) DestroyIcon(icon);
  }
}

HICON AppIconCache::ForWindow(HWND hwnd) {
  const std::wstring id = ReadAppUserModelId(hwnd);
  if (id.empty()) return nullptr;
  if (const auto it = icons_.find(id); it != icons_.end()) return it->second;
  // Failures are cached too, so a broken package isn't retried on every refresh.
  return icons_.emplace(id, LoadAppIcon(id)).first->second;
}

}  // namespace throwback
