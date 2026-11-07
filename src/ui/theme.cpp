#include "ui/theme.h"

namespace throwback::ui {
namespace {

// 8 pt at 96 DPI.
constexpr float kClassicSize = 11.0f;
// 14 pt at 96 DPI.
constexpr float kXpLargeSize = 18.67f;

}  // namespace

float FontSize(TextStyle style) {
  if (IsClassic(style)) return kClassicSize;
  if (style == TextStyle::XpBody || style == TextStyle::XpBold) return kClassicSize;
  if (style == TextStyle::XpUser || style == TextStyle::XpStart) return kXpLargeSize;
  return style == TextStyle::Title ? kTitleSize : kBodySize;
}

bool IsItalic(TextStyle style) { return style == TextStyle::Hint || style == TextStyle::XpStart; }

bool IsBold(TextStyle style) {
  return style == TextStyle::Classic98Bold || style == TextStyle::Classic2000Bold ||
         style == TextStyle::XpBold;
}

const wchar_t* FontFamily(TextStyle style) {
  switch (style) {
    case TextStyle::Classic98:
    case TextStyle::Classic98Bold:
      // The TrueType version of MS Sans Serif, which DirectWrite can draw.
      return L"Microsoft Sans Serif";
    case TextStyle::Classic2000:
    case TextStyle::Classic2000Bold:
    case TextStyle::XpBody:
    case TextStyle::XpBold:
      return L"Tahoma";
    case TextStyle::XpUser:
    case TextStyle::XpStart:
      return L"Franklin Gothic Medium";
    case TextStyle::Body:
    case TextStyle::Hint:
    case TextStyle::Title:
      break;
  }
  return L"Segoe UI";
}

bool IsClassic(TextStyle style) {
  return style == TextStyle::Classic98 || style == TextStyle::Classic98Bold ||
         style == TextStyle::Classic2000 || style == TextStyle::Classic2000Bold;
}

}  // namespace throwback::ui
