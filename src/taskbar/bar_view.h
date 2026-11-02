#pragma once

#include <windows.h>

#include <string_view>
#include <vector>

#include "core/layout.h"
#include "core/task_progress.h"
#include "ui/canvas.h"

namespace throwback {

struct TaskButtonModel {
  Rect rect;
  std::wstring_view title;
  HICON icon = nullptr;
  bool active = false;
  bool flashing = false;
  bool hovered = false;
  ProgressState progress = ProgressState::None;
  float progressFraction = 0.0f;
  HICON overlay = nullptr;  // a small status icon over the button's icon
};

struct TrayIconModel {
  Rect slot;  // the hover highlight
  Rect icon;
  HICON handle = nullptr;
  bool hovered = false;
};

// Everything shown on the bar for one frame.
struct BarModel {
  Rect bounds;
  bool top = false;  // docked to the top edge
  Rect startButton;
  bool startHovered = false;
  bool startOpen = false;
  std::vector<TaskButtonModel> buttons;
  std::vector<TrayIconModel> trayIcons;
  Rect clock;
  std::wstring_view time;
  std::wstring_view date;
  Rect showDesktop;
  bool showDesktopHovered = false;
};

// Draws the bar using UI kit components only.
void DrawBar(ui::Canvas& canvas, const BarModel& model);

}  // namespace throwback
