// The Windows XP components: Luna's start menu panels, rows and buttons,
// drawn from the theme's images with the colors of its NormalBlue.ini.
#include <algorithm>

#include "ui/canvas.h"

namespace throwback::ui {
namespace {

// At 96 DPI.
constexpr int kTile = 55;         // the white tile behind the account picture
constexpr int kTileInset = 3;     // where the picture sits on it
constexpr int kLogOffIcon = 24;   // the key and power icons
constexpr int kLogOffGap = 4;     // between such an icon and its label
constexpr int kShadowOffset = 2;  // of the user name and the Start label
constexpr int kFlagWidth = 25;
constexpr int kFlagHeight = 20;
constexpr int kStartLabelLeft = 10;  // the Start button's content margin

SkinImage ImageOf(XpPanel panel) {
  switch (panel) {
    case XpPanel::UserPane:
      return SkinImage::XpUserPane;
    case XpPanel::Programs:
      return SkinImage::XpPrograms;
    case XpPanel::MorePrograms:
      return SkinImage::XpMorePrograms;
    case XpPanel::Places:
      return SkinImage::XpPlaces;
    case XpPanel::LogOff:
      break;
  }
  return SkinImage::XpLogOff;
}

}  // namespace

void Canvas::XpBackground(const Rect& rect, XpPanel panel) {
  if (Skin(ImageOf(panel), 0, rect)) return;
  const bool light = panel == XpPanel::Programs || panel == XpPanel::MorePrograms;
  Fill(rect, light ? classic::kHighlight : xp::kProgramHot);
}

void Canvas::XpItem(const Rect& rect, XpList list, bool hovered) {
  if (!hovered) return;
  Fill(rect, list == XpList::Programs ? xp::kProgramHot : xp::kPlaceHot);
}

void Canvas::DrawShadowedText(const Rect& rect, std::wstring_view text, TextStyle style,
                              const Color& color, const Color& shadow) {
  IDWriteTextFormat* format = Format(style, TextAlign::Leading);
  const auto offset = static_cast<float>(Scale(kShadowOffset));
  DrawTextRun(rect, text, format, shadow, offset, offset);
  DrawTextRun(rect, text, format, color, 0.0f, 0.0f);
}

void Canvas::XpText(const Rect& rect, std::wstring_view text, XpTextVariant variant, bool hovered) {
  if (text.empty() || rect.Width() <= 0) return;
  switch (variant) {
    case XpTextVariant::User:
      DrawShadowedText(rect, text, TextStyle::XpUser, xp::kLightText, xp::kUserShadow);
      return;
    case XpTextVariant::LogOff:
      DrawTextRun(rect, text, Format(TextStyle::XpBody, TextAlign::Leading), xp::kLightText, 0.0f,
                  0.0f);
      return;
    case XpTextVariant::Program:
    case XpTextVariant::ProgramBold:
    case XpTextVariant::MorePrograms: {
      const bool bold = variant != XpTextVariant::Program;
      DrawTextRun(rect, text,
                  Format(bold ? TextStyle::XpBold : TextStyle::XpBody, TextAlign::Leading),
                  hovered ? xp::kLightText : xp::kProgramText, 0.0f, 0.0f);
      return;
    }
    case XpTextVariant::Place:
    case XpTextVariant::PlaceBold: {
      const bool bold = variant == XpTextVariant::PlaceBold;
      DrawTextRun(rect, text,
                  Format(bold ? TextStyle::XpBold : TextStyle::XpBody, TextAlign::Leading),
                  hovered ? xp::kLightText : xp::kPlaceText, 0.0f, 0.0f);
      return;
    }
  }
}

void Canvas::XpSeparator(const Rect& rect, XpList list) {
  const SkinImage image =
      list == XpList::Programs ? SkinImage::XpProgramsSeparator : SkinImage::XpPlacesSeparator;
  ID2D1Bitmap* bitmap = SkinBitmap(ResourceFor(PartOf(image), dpi_));
  if (!bitmap) return;
  const D2D1_SIZE_U size = bitmap->GetPixelSize();
  const int width = std::min(rect.Width(), Scale(static_cast<int>(size.width)));
  const int height = Scale(static_cast<int>(size.height));
  const int left = rect.left + (rect.Width() - width) / 2;
  const int top = rect.top + (rect.Height() - height) / 2;
  Skin(image, 0, {left, top, left + width, top + height});
}

void Canvas::XpUserPicture(const Rect& picture, const std::wstring& path,
                           std::wstring_view initial) {
  const int inset = Scale(kTileInset);
  const Rect tile{picture.left - inset, picture.top - inset, picture.left - inset + Scale(kTile),
                  picture.top - inset + Scale(kTile)};
  Skin(SkinImage::XpUserTile, 0, tile);
  if (ID2D1Bitmap* bitmap = PictureBitmap(path)) {
    target_->DrawBitmap(
        bitmap,
        D2D1::RectF(static_cast<float>(picture.left), static_cast<float>(picture.top),
                    static_cast<float>(picture.right), static_cast<float>(picture.bottom)),
        1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    return;
  }
  Fill(picture, xp::kPlaceHot);
  DrawShadowedText(picture, initial, TextStyle::XpUser, xp::kLightText, xp::kUserShadow);
}

void Canvas::XpMoreArrow(const Rect& rect, bool hovered) {
  Skin(hovered ? SkinImage::XpMoreArrowHot : SkinImage::XpMoreArrow, 0, rect);
}

void Canvas::XpLogOffButton(const Rect& rect, XpPowerButton button, std::wstring_view label,
                            bool hovered) {
  // The image holds three 24 px icons: undock, log off and turn off.
  const SkinImage image = hovered ? SkinImage::XpLogOffButtonsHot : SkinImage::XpLogOffButtons;
  ID2D1Bitmap* bitmap = SkinBitmap(ResourceFor(PartOf(image), dpi_));
  const int icon = Scale(kLogOffIcon);
  const int top = rect.top + (rect.Height() - icon) / 2;
  if (bitmap) {
    const auto column = static_cast<float>(button == XpPowerButton::LogOff ? 1 : 2);
    const auto source = static_cast<float>(kLogOffIcon);
    target_->DrawBitmap(
        bitmap,
        D2D1::RectF(static_cast<float>(rect.left), static_cast<float>(top),
                    static_cast<float>(rect.left + icon), static_cast<float>(top + icon)),
        1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
        D2D1::RectF(column * source, 0.0f, (column + 1.0f) * source, source));
  }
  XpText({rect.left + icon + Scale(kLogOffGap), rect.top, rect.right, rect.bottom}, label,
         XpTextVariant::LogOff, hovered);
}

void Canvas::XpStartButton(const Rect& rect, std::wstring_view label, OrbState state) {
  const int frame = state == OrbState::Pressed ? 2 : state == OrbState::Hot ? 1 : 0;
  if (!Skin(SkinImage::XpStartButton, frame, rect)) Fill(rect, xp::kProgramHot);
  const int flagLeft = rect.left + Scale(kStartLabelLeft);
  const int flagTop = rect.top + (rect.Height() - Scale(kFlagHeight)) / 2;
  Skin(SkinImage::XpFlag, 0,
       {flagLeft, flagTop, flagLeft + Scale(kFlagWidth), flagTop + Scale(kFlagHeight)});
  DrawShadowedText(
      {flagLeft + Scale(kFlagWidth) + Scale(kLogOffGap), rect.top, rect.right, rect.bottom}, label,
      TextStyle::XpStart, xp::kLightText, xp::kStartShadow);
}

}  // namespace throwback::ui
