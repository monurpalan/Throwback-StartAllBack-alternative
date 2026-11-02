#include "taskbar/app_icons.h"

#include <propsys.h>
#include <shellapi.h>
#include <wrl/client.h>

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

}  // namespace

HICON AppIconCache::ForWindow(HWND hwnd) {
  const std::wstring id = ReadAppUserModelId(hwnd);
  if (id.empty()) return nullptr;
  return icons_.Get(L"shell:AppsFolder\\" + id, kIconSize);
}

}  // namespace throwback
