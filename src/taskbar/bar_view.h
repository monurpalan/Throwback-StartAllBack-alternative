#pragma once

#include <windows.h>

#include <string_view>
#include <vector>

#include "core/bar_style.h"
#include "core/layout.h"
#include "core/options.h"
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
  bool top = false;       // docked to the top edge
  bool vertical = false;  // docked to the left or right edge
  Rect startButton;
  bool startHovered = false;
  bool startOpen = false;
  bool drawOrb = true;     // false while an OrbWindow shows the orb
  bool chosenOrb = false;  // an orb the user chose stands in for the style's button
  StartStyle startStyle = StartStyle::Win7;
  BarFamily family = BarFamily::Win7;
  bool labels = true;    // task buttons show titles
  bool dateLine = true;  // the clock shows the date too
  bool light = false;    // Windows 11 in light mode
  std::vector<TaskButtonModel> buttons;
  std::vector<TrayIconModel> trayIcons;
  Rect trayArrow;  // empty when no icon is hidden
  bool trayArrowHovered = false;
  bool trayOpen = false;  // the hidden icons are showing
  Rect clock;
  std::wstring_view time;
  std::wstring_view date;
  Rect showDesktop;
  bool showDesktopHovered = false;
  Rect notifications;  // upright bars only
  bool notificationsHovered = false;
};

// Draws the bar using UI kit components only.
ui::OrbState OrbStateFor(const BarModel& model);
void DrawStartButton(ui::Canvas& canvas, const BarModel& model);

void DrawBar(ui::Canvas& canvas, const BarModel& model);

}  // namespace throwback
