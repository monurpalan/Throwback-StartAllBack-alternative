#pragma once

#include <cstddef>
#include <vector>

namespace throwback {

struct Rect {
  int left = 0;
  int top = 0;
  int right = 0;
  int bottom = 0;

  int Width() const { return right - left; }
  int Height() const { return bottom - top; }
  bool Contains(int x, int y) const { return x >= left && x < right && y >= top && y < bottom; }
  bool operator==(const Rect&) const = default;
};

// Sizes in pixels at 96 DPI; use ScaleMetrics for other DPIs.
struct BarMetrics {
  int barWidth = 0;
  int barHeight = 0;
  int startButtonWidth = 0;
  int clockWidth = 0;
  int showDesktopWidth = 0;  // the slim button at the far end of the bar
  int trayWidth = 0;         // notification icons, left of the clock
  int minButtonWidth = 0;
  int maxButtonWidth = 0;
  int spacing = 0;
  int padding = 0;  // vertical inset of task buttons
};

struct BarLayout {
  Rect startButton;
  Rect clock;
  Rect tray;
  Rect showDesktop;
  // May hold fewer rects than requested when the buttons don't fit.
  std::vector<Rect> taskButtons;
};

// A size x size square centered in rect.
Rect CenteredSquare(const Rect& rect, int size);

int ScaleForDpi(int value, unsigned dpi);
BarMetrics ScaleMetrics(const BarMetrics& metrics, unsigned dpi);

// Places the start button on the left, the clock on the right and splits the
// space between them evenly across task buttons.
BarLayout ComputeLayout(const BarMetrics& metrics, std::size_t taskCount);

}  // namespace throwback
