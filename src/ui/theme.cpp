#include "ui/theme.h"

namespace throwback::ui {

float FontSize(TextStyle style) {
  switch (style) {
    case TextStyle::Title:
      return kTitleSize;
    case TextStyle::Subtitle:
      return kSubtitleSize;
    case TextStyle::Body:
    case TextStyle::Number:
      return kBodySize;
    case TextStyle::Small:
    case TextStyle::Label:
    case TextStyle::SmallNumber:
      return kSmallSize;
  }
  return kBodySize;
}

bool IsMono(TextStyle style) {
  return style == TextStyle::Label || style == TextStyle::Number || style == TextStyle::SmallNumber;
}

bool IsUppercase(TextStyle style) { return style == TextStyle::Label; }

Theme MakeTheme(bool light) {
  const Color base = light ? palette::kPaper : palette::kInk;
  const Color ink = light ? palette::kInk : palette::kPaper;
  return {
      .background = base,
      .foreground = ink,
      .hover = ink.WithAlpha(0.08f),
      .selected = ink.WithAlpha(0.14f),
      .selectedHover = ink.WithAlpha(0.20f),
      .attention = palette::kAccent.WithAlpha(0.40f),
      .accent = palette::kAccent,
  };
}

}  // namespace throwback::ui
