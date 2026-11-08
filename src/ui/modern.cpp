// The flat components of Windows 8 to 11: solid and acrylic surfaces,
// tiles, Segoe UI text and the icon fonts' symbols.
#include <algorithm>
#include <array>

#include "ui/canvas.h"

using Microsoft::WRL::ComPtr;

namespace throwback::ui {
namespace {

// At 96 DPI.
constexpr float kWin11Radius = 8.0f;
constexpr float kWin11FieldRadius = 16.0f;
constexpr float kHoverRadius = 4.0f;
constexpr int kFieldPadding = 16;
constexpr int kW10Logo = 16;
constexpr int kW81Logo = 32;
constexpr int kW11Logo = 24;
constexpr int kW11HoverInset = 4;
constexpr wchar_t kSearchGlyph = 0xE721;

bool IsWin11(ModernLook look) {
  return look == ModernLook::Win11Dark || look == ModernLook::Win11Light;
}

const Color& Win11TextOf(bool light, ModernTone tone) {
  if (light) {
    return tone == ModernTone::Secondary ? modern::kWin11LightSecondary
           : tone == ModernTone::Accent  ? modern::kWin11LightAccent
                                         : modern::kWin11LightText;
  }
  return tone == ModernTone::Secondary ? modern::kWin11DarkSecondary
         : tone == ModernTone::Accent  ? modern::kWin11DarkAccent
                                       : modern::kWin11DarkText;
}

const Color& TextOf(ModernLook look, ModernTone tone) {
  switch (look) {
    case ModernLook::Win8:
      return modern::kScreenText;
    case ModernLook::Win10:
      return tone == ModernTone::Secondary ? modern::kWin10Secondary : modern::kWin10Text;
    case ModernLook::Win11Dark:
      return Win11TextOf(false, tone);
    case ModernLook::Win11Light:
      break;
  }
  return Win11TextOf(true, tone);
}

const Color& HoverOf(ModernLook look) {
  switch (look) {
    case ModernLook::Win8:
      return modern::kTileHover;
    case ModernLook::Win10:
      return modern::kWin10Hover;
    case ModernLook::Win11Dark:
      return modern::kWin11DarkHover;
    case ModernLook::Win11Light:
      break;
  }
  return modern::kWin11LightHover;
}

constexpr std::array<Color, 6> kTiles{modern::kTileGreen,  modern::kTileBlue, modern::kTileOrange,
                                      modern::kTilePurple, modern::kTileTeal, modern::kTileRed};

Rect CenteredBox(const Rect& rect, int width, int height) {
  const int left = rect.left + (rect.Width() - width) / 2;
  const int top = rect.top + (rect.Height() - height) / 2;
  return {left, top, left + width, top + height};
}

}  // namespace

void Canvas::ModernSurface(const Rect& rect, ModernLook look) {
  switch (look) {
    case ModernLook::Win8:
      Fill(rect, modern::kScreen);
      return;
    case ModernLook::Win10:
      Fill(rect, modern::kWin10Surface);
      return;
    case ModernLook::Win11Dark:
      FillRound(rect, kWin11Radius, modern::kWin11DarkSurface);
      StrokeRound(rect, kWin11Radius, modern::kWin11DarkBorder);
      return;
    case ModernLook::Win11Light:
      FillRound(rect, kWin11Radius, modern::kWin11LightSurface);
      StrokeRound(rect, kWin11Radius, modern::kWin11LightBorder);
      return;
  }
}

void Canvas::ModernBar(const Rect& rect, ModernLook look) {
  const bool dark = look != ModernLook::Win11Light;
  Fill(rect, dark ? modern::kWin11DarkBar : modern::kWin11LightBar);
  Fill({rect.left, rect.top, rect.right, rect.top + std::max(1, Scale(1))},
       dark ? modern::kWin11DarkBorder : modern::kWin11LightBorder);
}

void Canvas::ModernHover(const Rect& rect, ModernLook look, bool hovered) {
  if (!hovered) return;
  if (IsWin11(look)) {
    FillRound(rect, kHoverRadius, HoverOf(look));
  } else {
    Fill(rect, HoverOf(look));
  }
}

void Canvas::TaskTile(const Rect& rect, bool light, TileState state) {
  // Like the Windows 7 task button StartAllBack draws: a glossy gradient,
  // a dark outer edge and a thin light line inside it.
  const float radius = static_cast<float>(Scale(3));
  const int line = std::max(1, Scale(1));
  const Rect inner{rect.left + line, rect.top + line, rect.right - line, rect.bottom - line};
  float lift = 0.0f;
  if (state == TileState::Hot) lift = 0.08f;
  if (state == TileState::Active) lift = 0.16f;
  if (state == TileState::Flashing) {
    FillGradient(rect, radius, FromRgb(0xFFC060, 0.85f), FromRgb(0xE07010, 0.85f));
  } else if (light) {
    FillGradient(rect, radius, FromRgb(0x000000, 0.04f + lift),
                 FromRgb(0x000000, 0.14f + lift));
  } else {
    FillGradient(rect, radius, FromRgb(0xFFFFFF, 0.30f + lift),
                 FromRgb(0xFFFFFF, 0.08f + lift));
  }
  StrokeRound(rect, radius, FromRgb(0x000000, light ? 0.30f : 0.60f));
  StrokeRound(inner, radius - 1.0f, FromRgb(0xFFFFFF, light ? 0.55f : 0.22f + lift));
}

void Canvas::ModernTile(const Rect& rect, int color, bool hovered) {
  Fill(rect, color < 0 ? modern::kWin10Accent : kTiles[static_cast<size_t>(color) % kTiles.size()]);
  if (hovered) Fill(rect, modern::kTileHover);
}

void Canvas::ModernText(const Rect& rect, std::wstring_view text, TextStyle style, ModernLook look,
                        ModernTone tone, TextAlign align) {
  if (text.empty() || rect.Width() <= 0) return;
  DrawTextRun(rect, text, Format(style, align), TextOf(look, tone), 0.0f, 0.0f);
}

void Canvas::ModernGlyph(const Rect& rect, wchar_t glyph, ModernLook look, ModernTone tone) {
  const TextStyle style = IsWin11(look) ? TextStyle::GlyphFluent : TextStyle::GlyphMdl2;
  DrawTextRun(rect, std::wstring_view(&glyph, 1), Format(style, TextAlign::Center),
              TextOf(look, tone), 0.0f, 0.0f);
}

void Canvas::ModernSearchBox(const Rect& rect, ModernLook look, std::wstring_view query,
                             std::wstring_view prompt) {
  const bool light = look == ModernLook::Win11Light;
  FillRound(rect, kWin11FieldRadius, light ? modern::kWin11LightField : modern::kWin11DarkField);
  StrokeRound(rect, kWin11FieldRadius,
              light ? modern::kWin11LightBorder : modern::kWin11DarkBorder);
  const int padding = Scale(kFieldPadding);
  const Rect glyph{rect.left + padding / 2, rect.top, rect.left + padding * 2, rect.bottom};
  ModernGlyph(glyph, kSearchGlyph, look, ModernTone::Secondary);
  const Rect text{glyph.right, rect.top, rect.right - padding, rect.bottom};
  if (query.empty()) {
    ModernText(text, prompt, TextStyle::Win11Body, look, ModernTone::Secondary);
  } else {
    ModernText(text, query, TextStyle::Win11Body, look, ModernTone::Primary);
  }
}

void Canvas::ModernPicture(const Rect& rect, const std::wstring& path, std::wstring_view initial,
                           ModernLook look) {
  const D2D1_ELLIPSE circle = D2D1::Ellipse(
      D2D1::Point2F(static_cast<float>(rect.left + rect.right) / 2.0f,
                    static_cast<float>(rect.top + rect.bottom) / 2.0f),
      static_cast<float>(rect.Width()) / 2.0f, static_cast<float>(rect.Height()) / 2.0f);
  ComPtr<ID2D1BitmapBrush> brush;
  if (ID2D1Bitmap* bitmap = PictureBitmap(path);
      bitmap && SUCCEEDED(target_->CreateBitmapBrush(bitmap, brush.GetAddressOf()))) {
    const D2D1_SIZE_F size = bitmap->GetSize();
    brush->SetTransform(
        D2D1::Matrix3x2F::Scale(static_cast<float>(rect.Width()) / size.width,
                                static_cast<float>(rect.Height()) / size.height) *
        D2D1::Matrix3x2F::Translation(static_cast<float>(rect.left), static_cast<float>(rect.top)));
    target_->FillEllipse(circle, brush.Get());
    return;
  }
  target_->FillEllipse(circle, Brush(look == ModernLook::Win11Light ? modern::kWin11LightBorder
                                                                    : modern::kWin10Hover));
  ModernText(rect, initial, TextStyle::Win11Header, look, ModernTone::Primary, TextAlign::Center);
}

void Canvas::ModernStartButton(const Rect& rect, ModernStartVariant variant, OrbState state) {
  const bool hot = state != OrbState::Normal;
  switch (variant) {
    case ModernStartVariant::Win81:
      if (hot) Skin(SkinImage::W81Glow, 0, rect);
      Skin(SkinImage::W81Logo, 0, CenteredBox(rect, Scale(kW81Logo), Scale(kW81Logo)));
      return;
    case ModernStartVariant::Win10:
      if (hot) Fill(rect, modern::kWin10Hover);
      Skin(SkinImage::W10Logo, 0, CenteredBox(rect, Scale(kW10Logo), Scale(kW10Logo)));
      return;
    case ModernStartVariant::Win11: {
      const int inset = Scale(kW11HoverInset);
      if (hot) {
        FillRound({rect.left + inset, rect.top + inset, rect.right - inset, rect.bottom - inset},
                  kHoverRadius, modern::kWin11DarkHover);
      }
      Skin(SkinImage::W11Logo, 0, CenteredBox(rect, Scale(kW11Logo), Scale(kW11Logo)));
      return;
    }
  }
}

void Canvas::ModernTaskbar(const Rect& rect, ModernLook look, bool top) {
  if (look == ModernLook::Win10) {
    Fill(rect, modern::kWin10Taskbar);
    return;
  }
  const bool light = look == ModernLook::Win11Light;
  Fill(rect, light ? modern::kWin11LightTaskbar : modern::kWin11DarkTaskbar);
  const int line = std::max(1, Scale(1));
  const Color& edge = light ? modern::kWin11LightBorder : modern::kWin11DarkBorder;
  Fill(top ? Rect{rect.left, rect.bottom - line, rect.right, rect.bottom}
           : Rect{rect.left, rect.top, rect.right, rect.top + line},
       edge);
}

void Canvas::ModernTaskButton(const Rect& rect, ModernLook look, bool active, bool flashing,
                              bool hovered) {
  const int line = std::max(1, Scale(2));
  if (look == ModernLook::Win10) {
    if (flashing) Fill(rect, modern::kFlashing);
    if (active) Fill(rect, modern::kWin10Active);
    if (hovered) Fill(rect, modern::kWin10Hover);
    Fill({rect.left, rect.bottom - line, rect.right, rect.bottom}, modern::kWin10Underline);
    return;
  }
  const bool light = look == ModernLook::Win11Light;
  if (active || hovered || flashing) {
    FillRound(rect, kHoverRadius,
              flashing ? modern::kFlashing
              : light  ? modern::kWin11LightHover
                       : modern::kWin11DarkHover);
  }
  // The pill: long and in the accent color for the active window.
  const int width = Scale(active ? 16 : 6);
  const int left = rect.left + (rect.Width() - width) / 2;
  const Rect pill{left, rect.bottom - Scale(5), left + width, rect.bottom - Scale(2)};
  FillRound(pill, 1.5f,
            active ? (light ? modern::kWin11LightAccent : modern::kWin11DarkAccent)
                   : modern::kWin11Running);
}

}  // namespace throwback::ui
