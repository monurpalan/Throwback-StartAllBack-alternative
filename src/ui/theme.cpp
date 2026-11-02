#include "ui/theme.h"

namespace throwback::ui {

float FontSize(TextStyle style) { return style == TextStyle::Title ? kTitleSize : kBodySize; }

bool IsItalic(TextStyle style) { return style == TextStyle::Hint; }

}  // namespace throwback::ui
