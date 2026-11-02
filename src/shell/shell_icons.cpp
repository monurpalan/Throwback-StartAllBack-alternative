#include "shell/shell_icons.h"

#include <shobjidl.h>
#include <wrl/client.h>

#include <vector>

using Microsoft::WRL::ComPtr;

namespace throwback::shell {
namespace {

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

}  // namespace

HICON LoadItemIcon(const std::wstring& parsingName, int size) {
  ComPtr<IShellItemImageFactory> factory;
  if (FAILED(SHCreateItemFromParsingName(parsingName.c_str(), nullptr,
                                         IID_PPV_ARGS(factory.GetAddressOf())))) {
    return nullptr;
  }
  HBITMAP bitmap = nullptr;
  if (FAILED(factory->GetImage({size, size}, SIIGBF_ICONONLY, &bitmap))) return nullptr;
  HICON icon = IconFromBitmap(bitmap);
  DeleteObject(bitmap);
  return icon;
}

IconCache::~IconCache() {
  for (const auto& [key, icon] : icons_) {
    if (icon) DestroyIcon(icon);
  }
}

HICON IconCache::Get(const std::wstring& parsingName, int size) {
  const std::wstring key = std::to_wstring(size) + L"|" + parsingName;
  if (const auto it = icons_.find(key); it != icons_.end()) return it->second;
  return icons_.emplace(key, LoadItemIcon(parsingName, size)).first->second;
}

}  // namespace throwback::shell
