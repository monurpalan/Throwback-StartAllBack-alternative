#include <algorithm>

#include "ui/canvas.h"

using Microsoft::WRL::ComPtr;

namespace throwback::ui {

void Canvas::Icon(const Rect& rect, IconKind kind) {
  switch (kind) {
    case IconKind::Start: {
      // Four-pane window logo. The gap is part of the glyph: 1/8 of its size.
      const float size = static_cast<float>(std::min(rect.Width(), rect.Height()));
      const float gap = size / 8.0f;
      const float pane = (size - gap) / 2.0f;
      const float left =
          static_cast<float>(rect.left) + (static_cast<float>(rect.Width()) - size) / 2.0f;
      const float top =
          static_cast<float>(rect.top) + (static_cast<float>(rect.Height()) - size) / 2.0f;
      brush_->SetColor(
          D2D1::ColorF(theme_.accent.r, theme_.accent.g, theme_.accent.b, theme_.accent.a));
      for (int row = 0; row < 2; ++row) {
        for (int column = 0; column < 2; ++column) {
          const float x = left + static_cast<float>(column) * (pane + gap);
          const float y = top + static_cast<float>(row) * (pane + gap);
          target_->FillRectangle(D2D1::RectF(x, y, x + pane, y + pane), brush_.Get());
        }
      }
      break;
    }
  }
}

void Canvas::Icon(const Rect& rect, HICON icon) {
  ID2D1Bitmap* bitmap = IconBitmap(icon);
  if (!bitmap) return;
  target_->DrawBitmap(bitmap,
                      D2D1::RectF(static_cast<float>(rect.left), static_cast<float>(rect.top),
                                  static_cast<float>(rect.right), static_cast<float>(rect.bottom)),
                      1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
}

ID2D1Bitmap* Canvas::IconBitmap(HICON icon) {
  if (!icon) return nullptr;
  usedIcons_.insert(icon);
  if (const auto it = iconCache_.find(icon); it != iconCache_.end()) return it->second.Get();

  ComPtr<IWICBitmap> wicBitmap;
  ComPtr<IWICFormatConverter> converter;
  ComPtr<ID2D1Bitmap> bitmap;
  if (FAILED(wic_->CreateBitmapFromHICON(icon, wicBitmap.GetAddressOf())) ||
      FAILED(wic_->CreateFormatConverter(converter.GetAddressOf())) ||
      FAILED(converter->Initialize(wicBitmap.Get(), GUID_WICPixelFormat32bppPBGRA,
                                   WICBitmapDitherTypeNone, nullptr, 0.0,
                                   WICBitmapPaletteTypeMedianCut)) ||
      FAILED(target_->CreateBitmapFromWicBitmap(converter.Get(), nullptr, bitmap.GetAddressOf()))) {
    return nullptr;
  }
  return iconCache_.emplace(icon, std::move(bitmap)).first->second.Get();
}

}  // namespace throwback::ui
