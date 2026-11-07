#include "ui/skin.h"

#include <algorithm>
#include <array>

#include "ui/canvas.h"
#include "ui/skin_ids.h"

using Microsoft::WRL::ComPtr;

namespace throwback::ui {
namespace {

// Frames and stretch edges as the Windows 7 theme (aero.msstyles) sets them.
constexpr std::array<SkinPart, 57> kParts{{
    {.resource = THROWBACK_SKIN_ORB_96, .frames = 3, .perDpi = true},
    {.resource = THROWBACK_SKIN_PICTURE_FRAME_96, .perDpi = true},
    {.resource = THROWBACK_SKIN_TASKBAR, .top = 12},
    {.resource = THROWBACK_SKIN_TASK_BUTTON,
     .frames = 10,
     .left = 3,
     .right = 3,
     .top = 3,
     .bottom = 3},
    {.resource = THROWBACK_SKIN_SHOW_DESKTOP,
     .frames = 3,
     .left = 2,
     .right = 2,
     .top = 15,
     .bottom = 2},
    {.resource = THROWBACK_SKIN_MENU_PROGRAMS, .left = 13, .right = 6, .top = 11},
    {.resource = THROWBACK_SKIN_MENU_SEARCH, .left = 24, .right = 19, .top = 23, .bottom = 29},
    {.resource = THROWBACK_SKIN_MENU_PLACES, .right = 8, .top = 8},
    {.resource = THROWBACK_SKIN_MENU_JUMP_LIST, .left = 1, .right = 12, .top = 11},
    {.resource = THROWBACK_SKIN_MENU_POWER, .right = 2, .top = 10, .bottom = 43},
    {.resource = THROWBACK_SKIN_SHUT_DOWN,
     .frames = 3,
     .left = 4,
     .right = 3,
     .top = 4,
     .bottom = 4},
    {.resource = THROWBACK_SKIN_ARROW, .frames = 3},
    {.resource = THROWBACK_SKIN_PANE_ITEM,
     .frames = 8,
     .left = 3,
     .right = 2,
     .top = 11,
     .bottom = 9},
    {.resource = THROWBACK_SKIN_GLASS_ITEM,
     .frames = 4,
     .left = 3,
     .right = 3,
     .top = 3,
     .bottom = 3},
    {.resource = THROWBACK_SKIN_TRAY_ARROW_96, .frames = 3, .perDpi = true},
    {.resource = THROWBACK_SKIN_TRAY_ARROW_OPEN_96, .frames = 3, .perDpi = true},
    {.resource = THROWBACK_SKIN_TRAY_ITEM,
     .frames = 3,
     .left = 2,
     .right = 2,
     .top = 14,
     .bottom = 14},
    {.resource = THROWBACK_SKIN_FLYOUT_LINK, .top = 5},
    {.resource = THROWBACK_SKIN_BANNER_98},
    {.resource = THROWBACK_SKIN_BANNER_ME},
    {.resource = THROWBACK_SKIN_BANNER_2000},
    {.resource = THROWBACK_SKIN_FLAG},
    {.resource = THROWBACK_SKIN_XP_START_BUTTON,
     .frames = 3,
     .left = 6,
     .right = 52,
     .top = 13,
     .bottom = 14},
    {.resource = THROWBACK_SKIN_XP_FLAG},
    {.resource = THROWBACK_SKIN_XP_USER_PANE, .left = 59, .right = 60, .top = 62},
    {.resource = THROWBACK_SKIN_XP_USER_TILE},
    {.resource = THROWBACK_SKIN_XP_PROGRAMS, .left = 2, .right = 153, .top = 3},
    {.resource = THROWBACK_SKIN_XP_PROGRAMS_SEPARATOR},
    {.resource = THROWBACK_SKIN_XP_MORE_PROGRAMS, .left = 3, .bottom = 1},
    {.resource = THROWBACK_SKIN_XP_MORE_ARROW},
    {.resource = THROWBACK_SKIN_XP_MORE_ARROW_HOT},
    {.resource = THROWBACK_SKIN_XP_PLACES, .left = 172, .right = 7, .top = 3, .bottom = 1},
    {.resource = THROWBACK_SKIN_XP_PLACES_SEPARATOR},
    {.resource = THROWBACK_SKIN_XP_LOGOFF, .left = 49, .right = 47, .bottom = 38},
    {.resource = THROWBACK_SKIN_XP_LOGOFF_BUTTONS},
    {.resource = THROWBACK_SKIN_XP_LOGOFF_BUTTONS_HOT},
    {.resource = THROWBACK_SKIN_XP_MENU, .left = 6, .right = 5, .top = 3, .bottom = 4},
    {.resource = THROWBACK_SKIN_XP_MENU_ITEM, .frames = 3, .left = 6, .right = 1},
    {.resource = THROWBACK_SKIN_XP_MENU_SEPARATOR, .left = 6, .right = 1, .top = 3},
    {.resource = THROWBACK_SKIN_VISTA_ORB_96, .frames = 3, .perDpi = true},
    {.resource = THROWBACK_SKIN_VISTA_PANE, .left = 13, .right = 6, .top = 13},
    {.resource = THROWBACK_SKIN_VISTA_MORE_PROGRAMS,
     .left = 13,
     .right = 6,
     .top = 4,
     .bottom = 18},
    {.resource = THROWBACK_SKIN_VISTA_PLACES, .right = 7, .top = 7, .bottom = 2},
    {.resource = THROWBACK_SKIN_VISTA_SEARCH, .left = 10, .right = 3, .top = 8, .bottom = 11},
    {.resource = THROWBACK_SKIN_VISTA_PROGRAM_HOVER, .left = 3, .right = 3, .top = 3, .bottom = 3},
    {.resource = THROWBACK_SKIN_VISTA_PLACE_HOVER, .left = 36, .right = 36, .top = 4, .bottom = 4},
    {.resource = THROWBACK_SKIN_VISTA_POWER},
    {.resource = THROWBACK_SKIN_VISTA_POWER_HOT},
    {.resource = THROWBACK_SKIN_VISTA_ARROW_BUTTON,
     .frames = 3,
     .left = 3,
     .right = 3,
     .top = 3,
     .bottom = 3},
    {.resource = THROWBACK_SKIN_VISTA_ARROW, .frames = 3},
    {.resource = THROWBACK_SKIN_VISTA_MORE_ARROW},
    {.resource = THROWBACK_SKIN_VISTA_MORE_ARROW_HOT},
    {.resource = THROWBACK_SKIN_VISTA_BACK_ARROW},
    {.resource = THROWBACK_SKIN_VISTA_BACK_ARROW_HOT},
    {.resource = THROWBACK_SKIN_VISTA_PICTURE_FRAME},
    {.resource = THROWBACK_SKIN_VISTA_PROGRAM_SEPARATOR},
    {.resource = THROWBACK_SKIN_VISTA_PLACE_SEPARATOR},
}};

D2D1_RECT_F RectF(float left, float top, float right, float bottom) {
  return D2D1::RectF(left, top, right, bottom);
}

}  // namespace

const SkinPart& PartOf(SkinImage image) { return kParts.at(static_cast<std::size_t>(image)); }

int ResourceFor(const SkinPart& part, unsigned dpi) {
  if (!part.perDpi) return part.resource;
  constexpr std::array<unsigned, 4> kDpis{96, 120, 144, 192};
  for (std::size_t i = 0; i < kDpis.size(); ++i) {
    if (dpi <= kDpis[i]) return part.resource + static_cast<int>(i);
  }
  return part.resource + static_cast<int>(kDpis.size()) - 1;
}

ComPtr<IWICBitmapSource> LoadSkinImage(IWICImagingFactory* wic, int resource) {
  const HMODULE module = GetModuleHandleW(nullptr);
  const HRSRC info = FindResourceW(module, MAKEINTRESOURCEW(resource), RT_RCDATA);
  const HGLOBAL data = info ? LoadResource(module, info) : nullptr;
  auto* bytes = static_cast<BYTE*>(data ? LockResource(data) : nullptr);
  ComPtr<IWICStream> stream;
  ComPtr<IWICBitmapDecoder> decoder;
  ComPtr<IWICBitmapFrameDecode> frame;
  ComPtr<IWICFormatConverter> converter;
  if (bytes && SUCCEEDED(wic->CreateStream(stream.GetAddressOf())) &&
      SUCCEEDED(stream->InitializeFromMemory(bytes, SizeofResource(module, info))) &&
      SUCCEEDED(wic->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnLoad,
                                             decoder.GetAddressOf())) &&
      SUCCEEDED(decoder->GetFrame(0, frame.GetAddressOf())) &&
      SUCCEEDED(wic->CreateFormatConverter(converter.GetAddressOf())) &&
      SUCCEEDED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA,
                                      WICBitmapDitherTypeNone, nullptr, 0.0,
                                      WICBitmapPaletteTypeMedianCut))) {
    return converter;
  }
  return nullptr;
}

ID2D1Bitmap* Canvas::SkinBitmap(int resource) {
  if (const auto it = skins_.find(resource); it != skins_.end()) return it->second.Get();

  ComPtr<ID2D1Bitmap> bitmap;
  if (const ComPtr<IWICBitmapSource> source = LoadSkinImage(wic_.Get(), resource)) {
    target_->CreateBitmapFromWicBitmap(source.Get(), nullptr, bitmap.GetAddressOf());
  }
  // A missing image is remembered too: the component then draws itself.
  return skins_.emplace(resource, bitmap).first->second.Get();
}

bool Canvas::HasSkin(SkinImage image) {
  return SkinBitmap(ResourceFor(PartOf(image), dpi_)) != nullptr;
}

bool Canvas::Skin(SkinImage image, int frame, const Rect& rect) {
  const SkinPart& part = PartOf(image);
  ID2D1Bitmap* bitmap = SkinBitmap(ResourceFor(part, dpi_));
  if (!bitmap) return false;
  if (rect.Width() <= 0 || rect.Height() <= 0) return true;

  const D2D1_SIZE_U size = bitmap->GetPixelSize();
  const float frameHeight = static_cast<float>(size.height) / static_cast<float>(part.frames);
  // A part with fewer frames than asked for shows its last one.
  const float sourceTop = frameHeight * static_cast<float>(std::min(frame, part.frames - 1));
  // Edges are in pixels of the file and drawn scaled for the DPI. (Per-DPI
  // images have no edges: they are drawn whole.)
  const std::array<float, 4> sourceX{
      0.0f, static_cast<float>(part.left),
      static_cast<float>(size.width) - static_cast<float>(part.right),
      static_cast<float>(size.width)};
  const std::array<float, 4> sourceY{sourceTop, sourceTop + static_cast<float>(part.top),
                                     sourceTop + frameHeight - static_cast<float>(part.bottom),
                                     sourceTop + frameHeight};
  const auto left = static_cast<float>(rect.left);
  const auto top = static_cast<float>(rect.top);
  const auto right = static_cast<float>(rect.right);
  const auto bottom = static_cast<float>(rect.bottom);
  const std::array<float, 4> destX{left, left + static_cast<float>(Scale(part.left)),
                                   right - static_cast<float>(Scale(part.right)), right};
  const std::array<float, 4> destY{top, top + static_cast<float>(Scale(part.top)),
                                   bottom - static_cast<float>(Scale(part.bottom)), bottom};
  // Stretching a one pixel strip with filtering would blend in its
  // neighbors, so 1:1 drawing samples exact pixels.
  const auto mode = dpi_ == 96 ? D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
                               : D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
  for (std::size_t row = 0; row < 3; ++row) {
    for (std::size_t column = 0; column < 3; ++column) {
      if (sourceX[column + 1] <= sourceX[column] || sourceY[row + 1] <= sourceY[row] ||
          destX[column + 1] <= destX[column] || destY[row + 1] <= destY[row]) {
        continue;
      }
      target_->DrawBitmap(
          bitmap, RectF(destX[column], destY[row], destX[column + 1], destY[row + 1]), 1.0f, mode,
          RectF(sourceX[column], sourceY[row], sourceX[column + 1], sourceY[row + 1]));
    }
  }
  return true;
}

}  // namespace throwback::ui
