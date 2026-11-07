// The classic components of Windows 98, Me and 2000: raised gray faces,
// blue selections and aliased 8 pt text.
#include <algorithm>

#include "ui/canvas.h"

namespace throwback::ui {
namespace {

// The flag of the Start button and its gap to the label, at 96 DPI.
constexpr int kFlagSize = 16;
constexpr int kFlagGap = 3;

const Color& FaceOf(ClassicLook look) {
  return look == ClassicLook::Win2000 ? classic::kFace2000 : classic::kFace98;
}

const Color& DarkShadowOf(ClassicLook look) {
  return look == ClassicLook::Win2000 ? classic::kDarkShadow2000 : classic::kDarkShadow98;
}

}  // namespace

void Canvas::ClassicFace(const Rect& rect, ClassicLook look, bool sunken) {
  // XP's menu: white with a blue edge, from the theme.
  if (look == ClassicLook::WinXp) {
    if (!Skin(SkinImage::XpMenu, 0, rect)) Fill(rect, classic::kHighlight);
    return;
  }
  const int line = std::max(1, Scale(1));
  const Color& face = FaceOf(look);
  Fill(rect, face);
  // Outer edge: face (top, left) and dark shadow (bottom, right); inner
  // edge: highlight and shadow. A sunken face swaps them.
  const Color& outerLight = sunken ? DarkShadowOf(look) : face;
  const Color& outerDark = sunken ? classic::kHighlight : DarkShadowOf(look);
  const Color& innerLight = sunken ? classic::kShadow : classic::kHighlight;
  const Color& innerDark = sunken ? face : classic::kShadow;
  const auto edge = [&](const Rect& r, const Color& light, const Color& dark) {
    Fill({r.left, r.top, r.right, r.top + line}, light);
    Fill({r.left, r.top, r.left + line, r.bottom}, light);
    Fill({r.left, r.bottom - line, r.right, r.bottom}, dark);
    Fill({r.right - line, r.top, r.right, r.bottom}, dark);
  };
  edge(rect, outerLight, outerDark);
  edge({rect.left + line, rect.top + line, rect.right - line, rect.bottom - line}, innerLight,
       innerDark);
}

void Canvas::ClassicBar(const Rect& rect, ClassicLook look, bool top) {
  const int line = std::max(1, Scale(1));
  Fill(rect, FaceOf(look));
  if (top) {
    Fill({rect.left, rect.bottom - 2 * line, rect.right, rect.bottom - line}, classic::kShadow);
    Fill({rect.left, rect.bottom - line, rect.right, rect.bottom}, DarkShadowOf(look));
  } else {
    Fill({rect.left, rect.top + line, rect.right, rect.top + 2 * line}, classic::kHighlight);
  }
}

void Canvas::ClassicWell(const Rect& rect, ClassicLook look) {
  const int line = std::max(1, Scale(1));
  Fill(rect, FaceOf(look));
  Fill({rect.left, rect.top, rect.right, rect.top + line}, classic::kShadow);
  Fill({rect.left, rect.top, rect.left + line, rect.bottom}, classic::kShadow);
  Fill({rect.left, rect.bottom - line, rect.right, rect.bottom}, classic::kHighlight);
  Fill({rect.right - line, rect.top, rect.right, rect.bottom}, classic::kHighlight);
}

void Canvas::ClassicItem(const Rect& rect, ClassicLook look, bool selected) {
  if (!selected) return;
  if (look == ClassicLook::WinXp) {
    if (!Skin(SkinImage::XpMenuItem, 1, rect)) Fill(rect, xp::kProgramHot);
    return;
  }
  Fill(rect, look == ClassicLook::Win98 ? classic::kSelection98 : classic::kSelection2000);
}

void Canvas::ClassicSeparator(const Rect& rect, ClassicLook look) {
  if (look == ClassicLook::WinXp && Skin(SkinImage::XpMenuSeparator, 0, rect)) return;
  const int line = std::max(1, Scale(1));
  const int middle = (rect.top + rect.bottom) / 2;
  Fill({rect.left, middle - line, rect.right, middle}, classic::kShadow);
  Fill({rect.left, middle, rect.right, middle + line}, classic::kHighlight);
}

void Canvas::ClassicText(const Rect& rect, std::wstring_view text, ClassicLook look, bool selected,
                         bool bold) {
  if (text.empty() || rect.Width() <= 0 || rect.Height() <= 0) return;
  if (look == ClassicLook::WinXp) {
    DrawTextRun(rect, text,
                Format(bold ? TextStyle::XpBold : TextStyle::XpBody, TextAlign::Leading),
                selected ? xp::kLightText : xp::kMenuText, 0.0f, 0.0f);
    return;
  }
  const TextStyle style = look == ClassicLook::Win98
                              ? (bold ? TextStyle::Classic98Bold : TextStyle::Classic98)
                              : (bold ? TextStyle::Classic2000Bold : TextStyle::Classic2000);
  // Those systems drew menu text without smoothing.
  target_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);
  DrawTextRun(rect, text, Format(style, TextAlign::Leading),
              selected ? classic::kSelectedText : classic::kText, 0.0f, 0.0f);
  target_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}

void Canvas::ClassicArrow(const Rect& rect, bool selected) {
  DrawChevron(rect, true, selected ? classic::kSelectedText : classic::kText);
}

void Canvas::ClassicBanner(const Rect& rect, SkinImage banner) {
  ID2D1Bitmap* bitmap = SkinBitmap(ResourceFor(PartOf(banner), dpi_));
  if (!bitmap) {
    Fill(rect, classic::kSelection98);
    return;
  }
  const D2D1_SIZE_U size = bitmap->GetPixelSize();
  const int height = std::min(rect.Height(), Scale(static_cast<int>(size.height)));
  // The strip above the banner continues its top color.
  Fill({rect.left, rect.top, rect.right, rect.bottom - height},
       banner == SkinImage::Banner98 ? classic::kBannerTop98 : classic::kBannerTop);
  Skin(banner, 0, {rect.left, rect.bottom - height, rect.right, rect.bottom});
}

void Canvas::ClassicStartButton(const Rect& rect, ClassicLook look, std::wstring_view label,
                                bool pressed) {
  ClassicFace(rect, look, pressed);
  const int shift = pressed ? std::max(1, Scale(1)) : 0;
  const int flag = Scale(kFlagSize);
  const int gap = Scale(kFlagGap);
  const int textWidth = MeasureText(
      label, look == ClassicLook::Win2000 ? TextStyle::Classic2000Bold : TextStyle::Classic98Bold);
  const int left = rect.left + (rect.Width() - flag - gap - textWidth) / 2 + shift;
  const int top = rect.top + (rect.Height() - flag) / 2 + shift;
  Skin(SkinImage::Flag, 0, {left, top, left + flag, top + flag});
  ClassicText({left + flag + gap, rect.top + shift, rect.right, rect.bottom + shift}, label, look,
              false, /*bold=*/true);
}

}  // namespace throwback::ui
