#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

#include "ui/canvas.h"

using Microsoft::WRL::ComPtr;

namespace throwback::ui {
namespace {

D2D1_COLOR_F ToD2D(const Color& color) { return D2D1::ColorF(color.r, color.g, color.b, color.a); }

D2D1_RECT_F ToRectF(const Rect& rect) {
  return D2D1::RectF(static_cast<float>(rect.left), static_cast<float>(rect.top),
                     static_cast<float>(rect.right), static_cast<float>(rect.bottom));
}

// The Windows 7 orb and arrow at 96 DPI; the orb's glow reaches past the bar.
constexpr int kOrbImageSize = 54;
constexpr int kArrowWidth = 6;
constexpr int kArrowHeight = 9;

Rect Centered(const Rect& rect, int width, int height) {
  const int left = rect.left + (rect.Width() - width) / 2;
  const int top = rect.top + (rect.Height() - height) / 2;
  return {left, top, left + width, top + height};
}

}  // namespace

void Canvas::Icon(const Rect& rect, IconKind kind) {
  switch (kind) {
    case IconKind::ChevronRight:
    case IconKind::ChevronLeft:
      DrawChevron(rect, kind == IconKind::ChevronRight, aero::kText);
      break;
    case IconKind::MenuArrow:
      if (!Skin(SkinImage::Arrow, 0, Centered(rect, Scale(kArrowWidth), Scale(kArrowHeight)))) {
        DrawChevron(rect, true, aero::kGlassText);
      }
      break;
    case IconKind::Search:
      DrawSearchGlyph(rect);
      break;
  }
}

void Canvas::DrawChevron(const Rect& rect, bool right, const Color& color) {
  // A small solid triangle, like the arrows of classic menus.
  const float size = static_cast<float>(Scale(kSpace8));
  const float cx = std::round(static_cast<float>(rect.left + rect.right) / 2.0f);
  const float cy = std::round(static_cast<float>(rect.top + rect.bottom) / 2.0f);
  const float tip = right ? size / 4.0f : -size / 4.0f;
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
  target_->FillGeometry(path.Get(), Brush(color));
}

void Canvas::DrawTriangle(const Rect& rect, bool up, const Color& color) {
  const float half = static_cast<float>(Scale(kSpace4));
  const float cx = std::round(static_cast<float>(rect.left + rect.right) / 2.0f);
  const float cy = std::round(static_cast<float>(rect.top + rect.bottom) / 2.0f);
  const float tip = up ? -half / 2.0f : half / 2.0f;
  ComPtr<ID2D1PathGeometry> path;
  ComPtr<ID2D1GeometrySink> sink;
  if (FAILED(factory_->CreatePathGeometry(path.GetAddressOf())) ||
      FAILED(path->Open(sink.GetAddressOf()))) {
    return;
  }
  sink->BeginFigure(D2D1::Point2F(cx - half, cy - tip), D2D1_FIGURE_BEGIN_FILLED);
  sink->AddLine(D2D1::Point2F(cx, cy + tip));
  sink->AddLine(D2D1::Point2F(cx + half, cy - tip));
  sink->EndFigure(D2D1_FIGURE_END_CLOSED);
  sink->Close();
  target_->FillGeometry(path.Get(), Brush(color));
}

void Canvas::DrawSearchGlyph(const Rect& rect) {
  // A magnifying glass: a ring with a short handle toward the bottom right.
  const float size = static_cast<float>(std::min(rect.Width(), rect.Height())) * 0.55f;
  const float cx = static_cast<float>(rect.left + rect.right) / 2.0f;
  const float cy = static_cast<float>(rect.top + rect.bottom) / 2.0f;
  const float radius = size * 0.32f;
  const float ringX = cx - size * 0.12f;
  const float ringY = cy - size * 0.12f;
  const float stroke = std::max(1.5f, static_cast<float>(Scale(kSpace2)) * 0.8f);
  ID2D1SolidColorBrush* brush = Brush(aero::kHintText);
  target_->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(ringX, ringY), radius, radius), brush, stroke);
  const float reach = radius * std::numbers::sqrt2_v<float> / 2.0f;
  target_->DrawLine(D2D1::Point2F(ringX + reach, ringY + reach),
                    D2D1::Point2F(cx + size * 0.45f, cy + size * 0.45f), brush, stroke * 1.3f);
}

void Canvas::Orb(const Rect& rect, OrbState state) {
  const int imageSize = Scale(kOrbImageSize);
  if (Skin(SkinImage::Orb, static_cast<int>(state), Centered(rect, imageSize, imageSize))) return;
  const float diameter = static_cast<float>(std::min(rect.Width(), rect.Height()));
  const float r = diameter / 2.0f;
  const float cx = static_cast<float>(rect.left + rect.right) / 2.0f;
  const float cy = static_cast<float>(rect.top + rect.bottom) / 2.0f;
  const D2D1_ELLIPSE sphere = D2D1::Ellipse(D2D1::Point2F(cx, cy), r - 0.5f, r - 0.5f);

  // The body: deep blue glass lit from below.
  const Color glow = state == OrbState::Hot       ? aero::kOrbHotGlow
                     : state == OrbState::Pressed ? aero::kOrbPressedGlow
                                                  : aero::kOrbGlow;
  const std::array<D2D1_GRADIENT_STOP, 3> bodyStops{
      {{0.0f, ToD2D(glow)}, {0.55f, ToD2D(aero::kOrbMiddle)}, {1.0f, ToD2D(aero::kOrbDeep)}}};
  ComPtr<ID2D1GradientStopCollection> bodyCollection;
  ComPtr<ID2D1RadialGradientBrush> body;
  if (SUCCEEDED(target_->CreateGradientStopCollection(bodyStops.data(), 3,
                                                      bodyCollection.GetAddressOf())) &&
      SUCCEEDED(target_->CreateRadialGradientBrush(
          D2D1::RadialGradientBrushProperties(D2D1::Point2F(cx, cy + r * 0.85f),
                                              D2D1::Point2F(0.0f, 0.0f), r * 1.25f, r * 1.25f),
          bodyCollection.Get(), body.GetAddressOf()))) {
    target_->FillEllipse(sphere, body.Get());
  }

  DrawFlag(cx, cy + r * 0.04f, r * 1.02f);

  // The gloss: a bright cap over the upper half.
  const std::array<D2D1_GRADIENT_STOP, 2> glossStops{
      {{0.0f, ToD2D(aero::kOrbGloss)}, {1.0f, ToD2D(aero::kOrbGloss.WithAlpha(0.02f))}}};
  ComPtr<ID2D1GradientStopCollection> glossCollection;
  ComPtr<ID2D1LinearGradientBrush> gloss;
  if (SUCCEEDED(target_->CreateGradientStopCollection(glossStops.data(), 2,
                                                      glossCollection.GetAddressOf())) &&
      SUCCEEDED(target_->CreateLinearGradientBrush(
          D2D1::LinearGradientBrushProperties(D2D1::Point2F(cx, cy - r),
                                              D2D1::Point2F(cx, cy + r * 0.05f)),
          glossCollection.Get(), gloss.GetAddressOf()))) {
    target_->FillEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy - r * 0.46f), r * 0.80f, r * 0.50f),
                         gloss.Get());
  }

  target_->DrawEllipse(sphere, Brush(aero::kOrbRim), 1.0f);
  target_->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), r - 1.5f, r - 1.5f),
                       Brush(aero::kGlassRim.WithAlpha(0.35f)), 1.0f);
}

void Canvas::DrawFlag(float cx, float cy, float size) {
  // The four-color flag, waving: every edge follows the same gentle curve.
  const float gap = std::max(1.0f, size / 18.0f);
  const float half = size / 2.0f;
  const float amplitude = size * 0.07f;
  const auto wave = [&](float x) {
    return -amplitude * std::sin(std::numbers::pi_v<float> * (x - (cx - half)) / size);
  };
  const std::array<Color, 4> colors{aero::kFlagRed, aero::kFlagGreen, aero::kFlagBlue,
                                    aero::kFlagYellow};
  for (int pane = 0; pane < 4; ++pane) {
    const float left = pane % 2 == 0 ? cx - half : cx + gap / 2.0f;
    const float right = pane % 2 == 0 ? cx - gap / 2.0f : cx + half;
    const float top = pane < 2 ? cy - half : cy + gap / 2.0f;
    const float bottom = pane < 2 ? cy - gap / 2.0f : cy + half;
    const float middle = (left + right) / 2.0f;
    const auto control = [&](float y) {
      return D2D1::Point2F(middle, y + 2.0f * wave(middle) - (wave(left) + wave(right)) / 2.0f);
    };

    ComPtr<ID2D1PathGeometry> path;
    ComPtr<ID2D1GeometrySink> sink;
    if (FAILED(factory_->CreatePathGeometry(path.GetAddressOf())) ||
        FAILED(path->Open(sink.GetAddressOf()))) {
      return;
    }
    sink->BeginFigure(D2D1::Point2F(left, top + wave(left)), D2D1_FIGURE_BEGIN_FILLED);
    sink->AddQuadraticBezier(
        D2D1::QuadraticBezierSegment(control(top), D2D1::Point2F(right, top + wave(right))));
    sink->AddLine(D2D1::Point2F(right, bottom + wave(right)));
    sink->AddQuadraticBezier(
        D2D1::QuadraticBezierSegment(control(bottom), D2D1::Point2F(left, bottom + wave(left))));
    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    sink->Close();
    target_->FillGeometry(path.Get(), Brush(colors[static_cast<size_t>(pane)]));
  }
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
  target_->DrawBitmap(bitmap, ToRectF(rect), 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
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
