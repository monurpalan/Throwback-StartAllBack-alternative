#pragma once

#include <cstddef>
#include <vector>

#include "core/options.h"

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
  Rect notifications;  // an upright bar's bell, under its clock
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

// An upright bar on the left or right edge, icons only, as StartAllBack
// draws one: a square start button on top, square task buttons stacked
// under it, and at the bottom the notification area, the clock (time over
// date), the notifications bell and the show desktop strip. clockHeight,
// trayHeight and bellHeight size those rows; showDesktopWidth is the strip's
// height.
BarLayout ComputeVerticalLayout(const BarMetrics& metrics, int clockHeight, int trayHeight,
                                int bellHeight, std::size_t taskCount);

// An upright bar at 96 DPI: wide enough for the date, as StartAllBack's;
// task buttons are inset and spaced like its tiles.
constexpr int kVerticalBarWidth = 56;
constexpr int kVerticalInset = 6;
constexpr int kVerticalSpacing = 4;
constexpr int kVerticalBell = 28;
constexpr int kVerticalShowDesktop = 12;

struct Point {
  int x = 0;
  int y = 0;
  bool operator==(const Point&) const = default;
};

// Where a popup of width x height opens for anchor (a button on a bar at
// barEdge): beside the bar, away from its edge, centered on the anchor
// along the bar and kept on screen.
Point PlacePopup(const Rect& anchor, int width, int height, Edge barEdge, int gap,
                 const Rect& screen);

}  // namespace throwback
