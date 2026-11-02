#include <algorithm>

#include "ui/canvas.h"

using Microsoft::WRL::ComPtr;

namespace throwback::ui {

void Canvas::Icon(const Rect& rect, IconKind kind) {
  switch (kind) {
    case IconKind::Start: {
      // The Windows logo: four square panes split by a thin cross, about a
      // sixteenth of the logo wide. Snapped to whole pixels so edges stay crisp.
      const int size = std::min(rect.Width(), rect.Height());
      const int gap = std::max(1, (size + 8) / 16);
      const int pane = (size - gap) / 2;
      const int total = 2 * pane + gap;
      const int left = rect.left + (rect.Width() - total) / 2;
      const int top = rect.top + (rect.Height() - total) / 2;
      for (int row = 0; row < 2; ++row) {
        for (int column = 0; column < 2; ++column) {
          const int x = left + column * (pane + gap);
          const int y = top + row * (pane + gap);
          Fill({x, y, x + pane, y + pane}, theme_.accent);
        }
      }
      break;
    }
    case IconKind::ChevronRight:
    case IconKind::ChevronLeft: {
      // A small solid triangle, like the arrows of classic menus.
      const float size = static_cast<float>(std::min(rect.Width(), rect.Height())) / 2.0f;
      const float cx = static_cast<float>(rect.left + rect.right) / 2.0f;
      const float cy = static_cast<float>(rect.top + rect.bottom) / 2.0f;
      const float tip = kind == IconKind::ChevronRight ? size / 2.0f : -size / 2.0f;
      ComPtr<ID2D1PathGeometry> path;
      ComPtr<ID2D1GeometrySink> sink;
      if (FAILED(factory_->CreatePathGeometry(path.GetAddressOf())) ||
          FAILED(path->Open(sink.GetAddressOf()))) {
        return;
      }
      sink->BeginFigure(D2D1::Point2F(cx - tip, cy - size / 2.0f), D2D1_FIGURE_BEGIN_FILLED);
      sink->AddLine(D2D1::Point2F(cx + tip, cy));
      sink->AddLine(D2D1::Point2F(cx - tip, cy + size / 2.0f));
      sink->EndFigure(D2D1_FIGURE_END_CLOSED);
      sink->Close();
      brush_->SetColor(D2D1::ColorF(theme_.foreground.r, theme_.foreground.g, theme_.foreground.b,
                                    theme_.foreground.a));
      target_->FillGeometry(path.Get(), brush_.Get());
      break;
    }
  }
}

void Canvas::Picture(const Rect& rect, const std::wstring& path) {
  ID2D1Bitmap* bitmap = PictureBitmap(path);
  if (!bitmap) return;
  target_->DrawBitmap(bitmap,
                      D2D1::RectF(static_cast<float>(rect.left), static_cast<float>(rect.top),
                                  static_cast<float>(rect.right), static_cast<float>(rect.bottom)),
                      1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
}

ID2D1Bitmap* Canvas::PictureBitmap(const std::wstring& path) {
  if (path.empty()) return nullptr;
  if (const auto it = pictures_.find(path); it != pictures_.end()) return it->second.Get();

  ComPtr<IWICBitmapDecoder> decoder;
  ComPtr<IWICBitmapFrameDecode> frame;
  ComPtr<IWICFormatConverter> converter;
  ComPtr<ID2D1Bitmap> bitmap;
  if (SUCCEEDED(wic_->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ,
                                                WICDecodeMetadataCacheOnDemand,
                                                decoder.GetAddressOf())) &&
      SUCCEEDED(decoder->GetFrame(0, frame.GetAddressOf())) &&
      SUCCEEDED(wic_->CreateFormatConverter(converter.GetAddressOf())) &&
      SUCCEEDED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA,
                                      WICBitmapDitherTypeNone, nullptr, 0.0,
                                      WICBitmapPaletteTypeMedianCut))) {
    target_->CreateBitmapFromWicBitmap(converter.Get(), nullptr, bitmap.GetAddressOf());
  }
  // A failed load is remembered too, so a broken file isn't read every frame.
  return pictures_.emplace(path, bitmap).first->second.Get();
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
