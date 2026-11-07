#include "ui/canvas.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

#include "ui/com_util.h"

using Microsoft::WRL::ComPtr;

namespace throwback::ui {
namespace {

D2D1_COLOR_F ToD2D(const Color& color) { return D2D1::ColorF(color.r, color.g, color.b, color.a); }

D2D1_RECT_F ToRectF(const Rect& rect) {
  return D2D1::RectF(static_cast<float>(rect.left), static_cast<float>(rect.top),
                     static_cast<float>(rect.right), static_cast<float>(rect.bottom));
}

// A one-pixel outline sits on pixel centers, half a pixel inside rect.
D2D1_ROUNDED_RECT OutlineShape(const Rect& rect, float radius) {
  return D2D1::RoundedRect(
      D2D1::RectF(static_cast<float>(rect.left) + 0.5f, static_cast<float>(rect.top) + 0.5f,
                  static_cast<float>(rect.right) - 0.5f, static_cast<float>(rect.bottom) - 0.5f),
      radius, radius);
}

}  // namespace

Canvas::Canvas(HWND hwnd) : hwnd_(hwnd) {
  ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory_.GetAddressOf()),
                "D2D1CreateFactory");
  ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                                    reinterpret_cast<IUnknown**>(dwrite_.GetAddressOf())),
                "DWriteCreateFactory");
  ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                 IID_PPV_ARGS(wic_.GetAddressOf())),
                "CoCreateInstance(WICImagingFactory)");
  fonts_ = std::make_unique<Fonts>(dwrite_.Get());
}

void Canvas::Resize(UINT width, UINT height) {
  if (target_) target_->Resize(D2D1::SizeU(width, height));
}

void Canvas::SetDpi(unsigned dpi) {
  if (dpi == dpi_) return;
  dpi_ = dpi;
  formats_.clear();
}

void Canvas::CreateDeviceResources() {
  if (target_) return;
  RECT client{};
  GetClientRect(hwnd_, &client);
  // Fixed 96 DPI so one Direct2D unit equals one pixel; Scale() handles DPI.
  // Premultiplied alpha lets DWM blend the frame over the blurred desktop.
  const auto properties = D2D1::RenderTargetProperties(
      D2D1_RENDER_TARGET_TYPE_DEFAULT,
      D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), 96.0f, 96.0f);
  const auto hwndProperties = D2D1::HwndRenderTargetProperties(
      hwnd_, D2D1::SizeU(static_cast<UINT32>(client.right - client.left),
                         static_cast<UINT32>(client.bottom - client.top)));
  ThrowIfFailed(
      factory_->CreateHwndRenderTarget(properties, hwndProperties, target_.GetAddressOf()),
      "CreateHwndRenderTarget");
  ThrowIfFailed(target_->CreateSolidColorBrush(ToD2D(aero::kText), brush_.GetAddressOf()),
                "CreateSolidColorBrush");
  target_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}

void Canvas::DiscardDeviceResources() {
  iconCache_.clear();
  pictures_.clear();
  skins_.clear();
  brush_.Reset();
  target_.Reset();
}

void Canvas::BeginFrame() {
  CreateDeviceResources();
  usedIcons_.clear();
  target_->BeginDraw();
  target_->Clear(D2D1::ColorF(0, 0, 0, 0));
}

void Canvas::EndFrame() {
  if (target_->EndDraw() == D2DERR_RECREATE_TARGET) {
    DiscardDeviceResources();
    return;
  }
  // Drop bitmaps of windows that are gone so the cache doesn't grow forever.
  std::erase_if(iconCache_,
                [this](const auto& entry) { return !usedIcons_.contains(entry.first); });
}

IDWriteTextFormat* Canvas::Format(TextStyle style, TextAlign align) {
  auto& format = formats_[{style, align}];
  if (!format) format = fonts_->CreateFormat(style, align, dpi_);
  return format.Get();
}

ID2D1SolidColorBrush* Canvas::Brush(const Color& color) {
  brush_->SetColor(ToD2D(color));
  return brush_.Get();
}

void Canvas::Fill(const Rect& rect, const Color& color) {
  if (rect.Width() <= 0 || rect.Height() <= 0) return;
  target_->FillRectangle(ToRectF(rect), Brush(color));
}

void Canvas::FillRound(const Rect& rect, float radius, const Color& color) {
  if (rect.Width() <= 0 || rect.Height() <= 0) return;
  target_->FillRoundedRectangle(D2D1::RoundedRect(ToRectF(rect), radius, radius), Brush(color));
}

void Canvas::FillGradient(const Rect& rect, float radius, const Color& top, const Color& bottom) {
  if (rect.Width() <= 0 || rect.Height() <= 0) return;
  const std::array<D2D1_GRADIENT_STOP, 2> stops{{{0.0f, ToD2D(top)}, {1.0f, ToD2D(bottom)}}};
  ComPtr<ID2D1GradientStopCollection> collection;
  ComPtr<ID2D1LinearGradientBrush> gradient;
  if (FAILED(target_->CreateGradientStopCollection(stops.data(), 2, collection.GetAddressOf())) ||
      FAILED(target_->CreateLinearGradientBrush(
          D2D1::LinearGradientBrushProperties(D2D1::Point2F(0.0f, static_cast<float>(rect.top)),
                                              D2D1::Point2F(0.0f, static_cast<float>(rect.bottom))),
          collection.Get(), gradient.GetAddressOf()))) {
    return;
  }
  target_->FillRoundedRectangle(D2D1::RoundedRect(ToRectF(rect), radius, radius), gradient.Get());
}

void Canvas::FillGlow(const Rect& rect, const Color& color) {
  // A soft light rising from the bottom edge, fading out toward the top.
  if (rect.Width() <= 0 || rect.Height() <= 0) return;
  const std::array<D2D1_GRADIENT_STOP, 2> stops{
      {{0.0f, ToD2D(color)}, {1.0f, ToD2D(color.WithAlpha(0.0f))}}};
  ComPtr<ID2D1GradientStopCollection> collection;
  ComPtr<ID2D1RadialGradientBrush> glow;
  const float cx = static_cast<float>(rect.left + rect.right) / 2.0f;
  if (FAILED(target_->CreateGradientStopCollection(stops.data(), 2, collection.GetAddressOf())) ||
      FAILED(target_->CreateRadialGradientBrush(
          D2D1::RadialGradientBrushProperties(
              D2D1::Point2F(cx, static_cast<float>(rect.bottom)), D2D1::Point2F(0.0f, 0.0f),
              static_cast<float>(rect.Width()) * 0.6f, static_cast<float>(rect.Height()) * 0.9f),
          collection.Get(), glow.GetAddressOf()))) {
    return;
  }
  target_->FillRoundedRectangle(D2D1::RoundedRect(ToRectF(rect), kCornerRadius, kCornerRadius),
                                glow.Get());
}

void Canvas::StrokeRound(const Rect& rect, float radius, const Color& color) {
  if (rect.Width() <= 1 || rect.Height() <= 1) return;
  target_->DrawRoundedRectangle(OutlineShape(rect, radius), Brush(color), 1.0f);
}

void Canvas::DrawTextRun(const Rect& rect, std::wstring_view text, IDWriteTextFormat* format,
                         const Color& color, float dx, float dy) {
  const D2D1_RECT_F area = ToRectF(rect);
  target_->DrawText(text.data(), static_cast<UINT32>(text.size()), format,
                    D2D1::RectF(area.left + dx, area.top + dy, area.right + dx, area.bottom + dy),
                    Brush(color), D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

void Canvas::Text(const Rect& rect, std::wstring_view text, TextStyle style, TextTone tone,
                  TextAlign align) {
  if (text.empty() || rect.Width() <= 0 || rect.Height() <= 0) return;
  IDWriteTextFormat* format = Format(style, align);
  switch (tone) {
    case TextTone::Dark:
      DrawTextRun(rect, text, format, aero::kText, 0.0f, 0.0f);
      break;
    case TextTone::Muted:
      DrawTextRun(rect, text, format, aero::kHintText, 0.0f, 0.0f);
      break;
    case TextTone::Light: {
      // White text on glass keeps readable over any wallpaper thanks to a
      // dark halo around the glyphs.
      constexpr std::array<std::pair<float, float>, 5> kHalo{
          {{-1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}}};
      for (const auto& [dx, dy] : kHalo) DrawTextRun(rect, text, format, aero::kTextShadow, dx, dy);
      DrawTextRun(rect, text, format, aero::kGlassText, 0.0f, 0.0f);
      break;
    }
  }
}

int Canvas::MeasureText(std::wstring_view text, TextStyle style) {
  return static_cast<int>(std::ceil(TextWidth(text, style)));
}

float Canvas::TextWidth(std::wstring_view text, TextStyle style) {
  ComPtr<IDWriteTextLayout> layout;
  if (FAILED(dwrite_->CreateTextLayout(text.data(), static_cast<UINT32>(text.size()),
                                       Format(style, TextAlign::Leading), 100000.0f, 1000.0f,
                                       layout.GetAddressOf()))) {
    return 0.0f;
  }
  DWRITE_TEXT_METRICS metrics{};
  layout->GetMetrics(&metrics);
  return metrics.widthIncludingTrailingWhitespace;
}

void Canvas::TextField(const Rect& rect, std::wstring_view text, std::wstring_view prompt,
                       bool focused) {
  // With the Windows 7 images the box is part of the menu's search strip.
  if (!HasSkin(SkinImage::MenuSearch)) {
    FillRound(rect, kCornerRadius, aero::kField);
    StrokeRound(rect, kCornerRadius, aero::kFieldBorder);
  }

  const int padding = Scale(kSpace4) + Scale(kSpace2);
  const int glyph = rect.Height();
  const Rect inner{rect.left + padding, rect.top, rect.right - glyph, rect.bottom};
  DrawSearchGlyph({rect.right - glyph, rect.top, rect.right, rect.bottom});
  if (text.empty()) {
    Text(inner, prompt, TextStyle::Hint, TextTone::Muted);
  } else {
    Text(inner, text, TextStyle::Body, TextTone::Dark);
  }
  if (!focused) return;
  // A thin caret after the text, as tall as a line of body text.
  const int caretX =
      inner.left + std::min(static_cast<int>(TextWidth(text, TextStyle::Body)), inner.Width());
  const int caretHeight = Scale(static_cast<int>(kBodySize) + kSpace4);
  const int caretTop = rect.top + (rect.Height() - caretHeight) / 2;
  Fill({caretX, caretTop, caretX + std::max(1, Scale(1)), caretTop + caretHeight}, aero::kText);
}

}  // namespace throwback::ui
