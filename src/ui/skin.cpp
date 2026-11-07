#include "ui/skin.h"

#include <array>

#include "ui/canvas.h"
#include "ui/skin_ids.h"

using Microsoft::WRL::ComPtr;

namespace throwback::ui {
namespace {

// Frames and stretch edges as the Windows 7 theme (aero.msstyles) sets them.
constexpr std::array<SkinPart, 14> kParts{{
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

ID2D1Bitmap* Canvas::SkinBitmap(int resource) {
  if (const auto it = skins_.find(resource); it != skins_.end()) return it->second.Get();

  ComPtr<ID2D1Bitmap> bitmap;
  const HMODULE module = GetModuleHandleW(nullptr);
  const HRSRC info = FindResourceW(module, MAKEINTRESOURCEW(resource), RT_RCDATA);
  const HGLOBAL data = info ? LoadResource(module, info) : nullptr;
  auto* bytes = static_cast<BYTE*>(data ? LockResource(data) : nullptr);
  ComPtr<IWICStream> stream;
  ComPtr<IWICBitmapDecoder> decoder;
  ComPtr<IWICBitmapFrameDecode> frame;
  ComPtr<IWICFormatConverter> converter;
  if (bytes && SUCCEEDED(wic_->CreateStream(stream.GetAddressOf())) &&
      SUCCEEDED(stream->InitializeFromMemory(bytes, SizeofResource(module, info))) &&
      SUCCEEDED(wic_->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnLoad,
                                              decoder.GetAddressOf())) &&
      SUCCEEDED(decoder->GetFrame(0, frame.GetAddressOf())) &&
      SUCCEEDED(wic_->CreateFormatConverter(converter.GetAddressOf())) &&
      SUCCEEDED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA,
                                      WICBitmapDitherTypeNone, nullptr, 0.0,
                                      WICBitmapPaletteTypeMedianCut))) {
    target_->CreateBitmapFromWicBitmap(converter.Get(), nullptr, bitmap.GetAddressOf());
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
  const float sourceTop = frameHeight * static_cast<float>(frame);
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
