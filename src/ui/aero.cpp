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

// Frames of the Windows 7 images (skin.h) for each state.
constexpr int kTaskFrame = 0;
constexpr int kTaskHotFrame = 1;
constexpr int kActiveTaskFrame = 3;
constexpr int kActiveTaskHotFrame = 4;
constexpr int kFlashingTaskFrame = 6;
constexpr int kHotFrame = 1;  // push buttons and program rows
// Program rows: the arrow half of a split row under the mouse, and a newly
// installed program.
constexpr int kItemPressedFrame = 2;
constexpr int kItemNewFrame = 3;
constexpr int kShowDesktopHotFrame = 2;
// The tray arrow at 96 DPI, drawn at its own size in the middle of its slot.
constexpr int kTrayArrowWidth = 21;
constexpr int kTrayArrowHeight = 20;

int TaskFrame(ButtonVariant variant, bool hovered) {
  if (variant == ButtonVariant::FlashingTask) return kFlashingTaskFrame;
  if (variant == ButtonVariant::ActiveTask) return hovered ? kActiveTaskHotFrame : kActiveTaskFrame;
  return hovered ? kTaskHotFrame : kTaskFrame;
}

}  // namespace

void Canvas::Glass(const Rect& rect, GlassVariant variant) {
  Fill(rect, aero::kGlassVeil);
  Fill(rect, aero::kGlassTint);
  // The Windows 7 images bring their own edges: the bar's, and the menu's
  // with its panels.
  if (variant != GlassVariant::Menu && Skin(SkinImage::Taskbar, 0, rect)) return;
  if (variant == GlassVariant::Menu && HasSkin(SkinImage::MenuPrograms)) return;
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

void Canvas::Panel(const Rect& rect, PanelVariant variant) {
  // Without the images only the white panes are drawn, inside the menu's
  // padding; the rest is plain glass.
  const int padding = Scale(kSpace8);
  Rect pane = rect;
  switch (variant) {
    case PanelVariant::Programs:
      if (Skin(SkinImage::MenuPrograms, 0, rect)) return;
      pane = {rect.left + padding, rect.top + padding, rect.right, rect.bottom};
      break;
    case PanelVariant::JumpList:
      if (Skin(SkinImage::MenuJumpList, 0, rect)) return;
      pane = {rect.left + padding, rect.top + padding, rect.right - padding, rect.bottom};
      break;
    case PanelVariant::Search:
      Skin(SkinImage::MenuSearch, 0, rect);
      return;
    case PanelVariant::Places:
      Skin(SkinImage::MenuPlaces, 0, rect);
      return;
    case PanelVariant::Power:
      Skin(SkinImage::MenuPower, 0, rect);
      return;
    case PanelVariant::FlyoutLink:
      if (!Skin(SkinImage::FlyoutLink, 0, rect)) Fill(rect, aero::kPane);
      return;
  }
  FillRound(pane, kCornerRadius, aero::kPane);
  StrokeRound(pane, kCornerRadius, aero::kPaneBorder);
}

void Canvas::TaskButton(const Rect& rect, ButtonVariant variant, bool hovered) {
  if (Skin(SkinImage::TaskButton, TaskFrame(variant, hovered), rect)) {
    // Windows 7 lights the hovered button with a glow from below.
    if (hovered) FillGlow(rect, aero::kHotGlow);
    return;
  }
  if (variant == ButtonVariant::FlashingTask) {
    FillGradient(rect, kCornerRadius, aero::kFlashTop, aero::kFlashBottom);
  } else if (variant == ButtonVariant::ActiveTask) {
    FillGradient(rect, kCornerRadius, aero::kActiveTaskTop, aero::kActiveTaskBottom);
  } else {
    FillGradient(rect, kCornerRadius, aero::kTaskTop, aero::kTaskBottom);
  }
  if (hovered) FillGlow(rect, aero::kHotGlow);
  StrokeRound(rect, kCornerRadius, aero::kGlassItemBorder);
  StrokeRound(Inset(rect, std::max(1, Scale(1))), kCornerRadius - 1.0f, aero::kTaskInner);
}

void Canvas::TrayArrow(const Rect& rect, bool open, bool hovered) {
  const int left = rect.left + (rect.Width() - Scale(kTrayArrowWidth)) / 2;
  const int top = rect.top + (rect.Height() - Scale(kTrayArrowHeight)) / 2;
  const Rect button{left, top, left + Scale(kTrayArrowWidth), top + Scale(kTrayArrowHeight)};
  if (Skin(open ? SkinImage::TrayArrowOpen : SkinImage::TrayArrow, hovered ? kHotFrame : 0,
           button)) {
    return;
  }
  Button(button, ButtonVariant::GlassItem, hovered);
  DrawTriangle(button, !open, aero::kGlassText);
}

void Canvas::CommandButton(const Rect& button, bool arrow, bool hovered) {
  const int frame = hovered ? kHotFrame : 0;
  // The menu half shares its left border with the button it is joined to,
  // the way Windows 7 draws the split shut down button.
  const Rect rect =
      arrow ? Rect{button.left - std::max(1, Scale(1)), button.top, button.right, button.bottom}
            : button;
  if (arrow && Skin(SkinImage::PowerArrow, frame, rect)) return;
  if (Skin(SkinImage::ShutDown, frame, rect)) return;
  // Gloss on the upper half, a deeper body below that lights up blue on
  // hover.
  const int line = std::max(1, Scale(1));
  const Rect inner = Inset(rect, line);
  const int middle = (rect.top + rect.bottom) / 2;
  FillGradient({inner.left, inner.top, inner.right, middle}, kCornerRadius - 1.0f,
               aero::kCommandGlossTop, aero::kCommandGlossBottom);
  FillGradient({inner.left, middle, inner.right, inner.bottom}, kCornerRadius - 1.0f,
               hovered ? aero::kCommandHotTop : aero::kCommandBodyTop,
               hovered ? aero::kCommandHotBottom : aero::kCommandBodyBottom);
  StrokeRound(rect, kCornerRadius, aero::kCommandBorder);
  StrokeRound(inner, kCornerRadius - 1.0f, aero::kItemInner);
}

void Canvas::SplitMenuItem(const Rect& rect, int arrowWidth, bool arrowHot, bool fresh) {
  const Rect main{rect.left, rect.top, rect.right - arrowWidth, rect.bottom};
  const Rect arrow{rect.right - arrowWidth, rect.top, rect.right, rect.bottom};
  // The whole row is lit, the arrow half darker while the mouse is on it;
  // a new program's half away from the mouse stays orange.
  const int mainFrame = arrowHot && fresh ? kItemNewFrame : kHotFrame;
  if (Skin(SkinImage::SplitMain, mainFrame, main) &&
      Skin(SkinImage::SplitArrow, arrowHot ? kItemPressedFrame : kHotFrame, arrow)) {
    return;
  }
  Button(rect, ButtonVariant::MenuItem, true);
  const int line = std::max(1, Scale(1));
  Fill({arrow.left, arrow.top + line * 2, arrow.left + line, arrow.bottom - line * 2},
       aero::kItemBorder);
}

void Canvas::Button(const Rect& rect, ButtonVariant variant, bool hovered) {
  const int line = std::max(1, Scale(1));
  switch (variant) {
    case ButtonVariant::MenuItem:
      if (!hovered || Skin(SkinImage::ProgramItem, kHotFrame, rect)) return;
      FillGradient(rect, kCornerRadius, aero::kItemTop, aero::kItemBottom);
      StrokeRound(rect, kCornerRadius, aero::kItemBorder);
      StrokeRound(Inset(rect, line), kCornerRadius - 1.0f, aero::kItemInner);
      return;
    case ButtonVariant::NewMenuItem:
      if (Skin(SkinImage::ProgramItem, hovered ? kHotFrame : kItemNewFrame, rect)) return;
      if (hovered) {
        Button(rect, ButtonVariant::MenuItem, true);
        return;
      }
      FillGradient(rect, kCornerRadius, aero::kNewItemTop, aero::kNewItemBottom);
      StrokeRound(rect, kCornerRadius, aero::kNewItemBorder);
      StrokeRound(Inset(rect, line), kCornerRadius - 1.0f, aero::kItemInner);
      return;
    case ButtonVariant::GlassItem:
      if (!hovered || Skin(SkinImage::PlaceItem, 0, rect)) return;
      FillGradient(rect, kCornerRadius, aero::kGlassItemTop, aero::kGlassItemBottom);
      StrokeRound(rect, kCornerRadius, aero::kGlassItemBorder);
      StrokeRound(Inset(rect, line), kCornerRadius - 1.0f, aero::kGlassItemInner);
      return;
    case ButtonVariant::Task:
    case ButtonVariant::ActiveTask:
    case ButtonVariant::FlashingTask:
      TaskButton(rect, variant, hovered);
      return;
    case ButtonVariant::Command:
    case ButtonVariant::CommandArrow:
      CommandButton(rect, variant == ButtonVariant::CommandArrow, hovered);
      return;
    case ButtonVariant::TrayItem:
      if (!hovered || Skin(SkinImage::TrayItem, kHotFrame, rect)) return;
      Button(rect, ButtonVariant::GlassItem, hovered);
      return;
    case ButtonVariant::TrayArrow:
    case ButtonVariant::TrayArrowOpen:
      TrayArrow(rect, variant == ButtonVariant::TrayArrowOpen, hovered);
      return;
    case ButtonVariant::ShowDesktop:
      if (Skin(SkinImage::ShowDesktop, hovered ? kShowDesktopHotFrame : 0, rect)) return;
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
  // Windows 7's lines fade out at both ends; they keep their own size,
  // centered on the line and never wider than it.
  const SkinImage line =
      variant == SeparatorVariant::Pane ? SkinImage::ProgramSeparator : SkinImage::PlaceSeparator;
  if (ID2D1Bitmap* bitmap = SkinBitmap(ResourceFor(PartOf(line, skinSet_), dpi_))) {
    const D2D1_SIZE_U size = bitmap->GetPixelSize();
    const int width = std::min(rect.Width(), Scale(static_cast<int>(size.width)));
    const int height = std::max(1, Scale(static_cast<int>(size.height)));
    const int left = rect.left + (rect.Width() - width) / 2;
    Skin(line, 0, {left, rect.top, left + width, rect.top + height});
    return;
  }
  if (variant == SeparatorVariant::Pane) {
    Fill(rect, aero::kPaneSeparator);
    return;
  }
  // An etched line on glass: dark, then light under it.
  Fill(rect, aero::kGlassEdge.WithAlpha(0.35f));
  Fill({rect.left, rect.bottom, rect.right, rect.bottom + rect.Height()},
       aero::kGlassRim.WithAlpha(0.18f));
}

void Canvas::ImageCell(SkinImage image, int index, int count, const Rect& rect) {
  ID2D1Bitmap* bitmap = SkinBitmap(ResourceFor(PartOf(image, skinSet_), dpi_));
  if (!bitmap || count <= 0) return;
  const D2D1_SIZE_U size = bitmap->GetPixelSize();
  const float cell = static_cast<float>(size.width) / static_cast<float>(count);
  target_->DrawBitmap(
      bitmap,
      D2D1::RectF(static_cast<float>(rect.left), static_cast<float>(rect.top),
                  static_cast<float>(rect.right), static_cast<float>(rect.bottom)),
      1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
      D2D1::RectF(cell * static_cast<float>(index), 0.0f, cell * static_cast<float>(index + 1),
                  static_cast<float>(size.height)));
}

void Canvas::AccountPicture(const Rect& rect, const std::wstring& path, std::wstring_view initial,
                            SkinImage frame, int frameInset) {
  // A glossy frame around the picture, as on the Windows 7 start menu.
  if (!Skin(frame, 0, Inset(rect, -Scale(frameInset)))) {
    const Rect drawn = Inset(rect, -Scale(kSpace4));
    FillGradient(drawn, kCornerRadius, aero::kGlassItemInner, aero::kGlassItemBottom);
    StrokeRound(drawn, kCornerRadius, aero::kGlassItemBorder);
    StrokeRound(Inset(drawn, std::max(1, Scale(1))), kCornerRadius - 1.0f, aero::kGlassItemInner);
  }

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
