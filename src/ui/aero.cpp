// The Aero surfaces of the UI kit: glass, the white pane, buttons and
// separators, drawn the way Windows 7 draws them.
#include <algorithm>

#include "ui/canvas.h"

namespace throwback::ui {
namespace {

// DWM rounds glass menus with this radius, so the outline follows it.
constexpr float kMenuRadius = 8.0f;

Rect Inset(const Rect& rect, int amount) {
  return {rect.left + amount, rect.top + amount, rect.right - amount, rect.bottom - amount};
}

}  // namespace

void Canvas::Glass(const Rect& rect, GlassVariant variant) {
  Fill(rect, aero::kGlassVeil);
  Fill(rect, aero::kGlassTint);
  const int line = std::max(1, Scale(1));
  switch (variant) {
    case GlassVariant::BarBottom: {
      // Light catches the upper half; the top edge is a dark line over a
      // bright one.
      FillGradient({rect.left, rect.top, rect.right, (rect.top + rect.bottom) / 2}, 0.0f,
                   aero::kGlassSheen, aero::kGlassSheen.WithAlpha(0.0f));
      Fill({rect.left, rect.top, rect.right, rect.top + line}, aero::kGlassEdge);
      Fill({rect.left, rect.top + line, rect.right, rect.top + 2 * line}, aero::kGlassRim);
      break;
    }
    case GlassVariant::BarTop: {
      FillGradient({rect.left, rect.top, rect.right, (rect.top + rect.bottom) / 2}, 0.0f,
                   aero::kGlassSheen, aero::kGlassSheen.WithAlpha(0.0f));
      Fill({rect.left, rect.bottom - line, rect.right, rect.bottom}, aero::kGlassEdge);
      Fill({rect.left, rect.bottom - 2 * line, rect.right, rect.bottom - line}, aero::kGlassRim);
      break;
    }
    case GlassVariant::Menu:
      FillGradient({rect.left, rect.top, rect.right, rect.top + Scale(kSpace24) * 3}, kMenuRadius,
                   aero::kGlassSheen, aero::kGlassSheen.WithAlpha(0.0f));
      StrokeRound(rect, kMenuRadius, aero::kGlassEdge);
      StrokeRound(Inset(rect, line), kMenuRadius - 1.0f, aero::kGlassRim.WithAlpha(0.30f));
      break;
  }
}

void Canvas::Panel(const Rect& rect) {
  FillRound(rect, kCornerRadius, aero::kPane);
  StrokeRound(rect, kCornerRadius, aero::kPaneBorder);
}

void Canvas::Button(const Rect& rect, ButtonVariant variant, bool hovered) {
  const int line = std::max(1, Scale(1));
  switch (variant) {
    case ButtonVariant::MenuItem:
      if (!hovered) return;
      FillGradient(rect, kCornerRadius, aero::kItemTop, aero::kItemBottom);
      StrokeRound(rect, kCornerRadius, aero::kItemBorder);
      StrokeRound(Inset(rect, line), kCornerRadius - 1.0f, aero::kItemInner);
      return;
    case ButtonVariant::GlassItem:
      if (!hovered) return;
      FillGradient(rect, kCornerRadius, aero::kGlassItemTop, aero::kGlassItemBottom);
      StrokeRound(rect, kCornerRadius, aero::kGlassItemBorder);
      StrokeRound(Inset(rect, line), kCornerRadius - 1.0f, aero::kGlassItemInner);
      return;
    case ButtonVariant::Task:
    case ButtonVariant::ActiveTask:
    case ButtonVariant::FlashingTask: {
      if (variant == ButtonVariant::FlashingTask) {
        FillGradient(rect, kCornerRadius, aero::kFlashTop, aero::kFlashBottom);
      } else if (variant == ButtonVariant::ActiveTask) {
        FillGradient(rect, kCornerRadius, aero::kActiveTaskTop, aero::kActiveTaskBottom);
      } else {
        FillGradient(rect, kCornerRadius, aero::kTaskTop, aero::kTaskBottom);
      }
      if (hovered) FillGlow(rect, aero::kHotGlow);
      StrokeRound(rect, kCornerRadius, aero::kGlassItemBorder);
      StrokeRound(Inset(rect, line), kCornerRadius - 1.0f, aero::kTaskInner);
      return;
    }
    case ButtonVariant::Command: {
      // Gloss on the upper half, a deeper body below that lights up blue on
      // hover.
      const Rect inner = Inset(rect, line);
      const int middle = (rect.top + rect.bottom) / 2;
      FillGradient({inner.left, inner.top, inner.right, middle}, kCornerRadius - 1.0f,
                   aero::kCommandGlossTop, aero::kCommandGlossBottom);
      FillGradient({inner.left, middle, inner.right, inner.bottom}, kCornerRadius - 1.0f,
                   hovered ? aero::kCommandHotTop : aero::kCommandBodyTop,
                   hovered ? aero::kCommandHotBottom : aero::kCommandBodyBottom);
      StrokeRound(rect, kCornerRadius, aero::kCommandBorder);
      StrokeRound(inner, kCornerRadius - 1.0f, aero::kItemInner);
      return;
    }
    case ButtonVariant::ShowDesktop:
      FillGradient(rect, 0.0f, aero::kTaskTop, aero::kTaskBottom);
      if (hovered) Fill(rect, aero::kGlassItemTop);
      Fill({rect.left, rect.top, rect.left + line, rect.bottom}, aero::kGlassEdge);
      Fill({rect.left + line, rect.top, rect.left + 2 * line, rect.bottom}, aero::kTaskInner);
      return;
  }
}

void Canvas::Progress(const Rect& rect, ProgressVariant variant, float fraction) {
  const int line = std::max(1, Scale(1));
  const Rect inner = Inset(rect, line);
  switch (variant) {
    case ProgressVariant::Indeterminate:
      // Without a value, a faint fill says "busy" over the whole button.
      FillGradient(inner, kCornerRadius - 1.0f, aero::kProgressTop.WithAlpha(0.30f),
                   aero::kProgressBottom.WithAlpha(0.30f));
      return;
    case ProgressVariant::Normal:
    case ProgressVariant::Paused:
    case ProgressVariant::Error: {
      const int width = static_cast<int>(static_cast<float>(inner.Width()) * fraction);
      const Rect filled{inner.left, inner.top, inner.left + width, inner.bottom};
      const Color top = variant == ProgressVariant::Normal   ? aero::kProgressTop
                        : variant == ProgressVariant::Paused ? aero::kPausedTop
                                                             : aero::kErrorTop;
      const Color bottom = variant == ProgressVariant::Normal   ? aero::kProgressBottom
                           : variant == ProgressVariant::Paused ? aero::kPausedBottom
                                                                : aero::kErrorBottom;
      FillGradient(filled, kCornerRadius - 1.0f, top, bottom);
      return;
    }
  }
}

void Canvas::Separator(const Rect& rect, SeparatorVariant variant) {
  if (variant == SeparatorVariant::Pane) {
    Fill(rect, aero::kPaneSeparator);
    return;
  }
  // An etched line on glass: dark, then light under it.
  Fill(rect, aero::kGlassEdge.WithAlpha(0.35f));
  Fill({rect.left, rect.bottom, rect.right, rect.bottom + rect.Height()},
       aero::kGlassRim.WithAlpha(0.18f));
}

void Canvas::AccountPicture(const Rect& rect, const std::wstring& path, std::wstring_view initial) {
  // A glossy frame around the picture, as on the Windows 7 start menu.
  const Rect frame = Inset(rect, -Scale(kSpace4));
  FillGradient(frame, kCornerRadius, aero::kGlassItemInner, aero::kGlassItemBottom);
  StrokeRound(frame, kCornerRadius, aero::kGlassItemBorder);
  StrokeRound(Inset(frame, std::max(1, Scale(1))), kCornerRadius - 1.0f, aero::kGlassItemInner);

  if (ID2D1Bitmap* bitmap = PictureBitmap(path)) {
    target_->DrawBitmap(
        bitmap,
        D2D1::RectF(static_cast<float>(rect.left), static_cast<float>(rect.top),
                    static_cast<float>(rect.right), static_cast<float>(rect.bottom)),
        1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    return;
  }
  FillGradient(rect, 0.0f, aero::kOrbMiddle, aero::kOrbDeep);
  Text(rect, initial, TextStyle::Title, TextTone::Light, TextAlign::Center);
}

}  // namespace throwback::ui
