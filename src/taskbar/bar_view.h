#pragma once

#include <windows.h>

#include <string_view>
#include <vector>

#include "core/layout.h"
#include "ui/canvas.h"

namespace throwback {

struct TaskButtonModel {
  Rect rect;
  std::wstring_view title;
  HICON icon = nullptr;
  bool active = false;
  bool flashing = false;
  bool hovered = false;
};

// Everything shown on the bar for one frame.
struct BarModel {
  Rect startButton;
  bool startHovered = false;
  std::vector<TaskButtonModel> buttons;
  Rect clock;
  std::wstring_view time;
  std::wstring_view date;
  Rect showDesktop;
  bool showDesktopHovered = false;
};

// Draws the bar using UI kit components only.
void DrawBar(ui::Canvas& canvas, const BarModel& model);

}  // namespace throwback
