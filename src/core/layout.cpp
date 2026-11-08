#include "core/layout.h"

#include <algorithm>

namespace throwback {

Rect CenteredSquare(const Rect& rect, int size) {
  const int left = rect.left + (rect.Width() - size) / 2;
  const int top = rect.top + (rect.Height() - size) / 2;
  return {left, top, left + size, top + size};
}

int ScaleForDpi(int value, unsigned dpi) { return (value * static_cast<int>(dpi) + 48) / 96; }

BarMetrics ScaleMetrics(const BarMetrics& metrics, unsigned dpi) {
  return {
      .barWidth = ScaleForDpi(metrics.barWidth, dpi),
      .barHeight = ScaleForDpi(metrics.barHeight, dpi),
      .startButtonWidth = ScaleForDpi(metrics.startButtonWidth, dpi),
      .clockWidth = ScaleForDpi(metrics.clockWidth, dpi),
      .showDesktopWidth = ScaleForDpi(metrics.showDesktopWidth, dpi),
      .trayWidth = ScaleForDpi(metrics.trayWidth, dpi),
      .minButtonWidth = ScaleForDpi(metrics.minButtonWidth, dpi),
      .maxButtonWidth = ScaleForDpi(metrics.maxButtonWidth, dpi),
      .spacing = ScaleForDpi(metrics.spacing, dpi),
      .padding = ScaleForDpi(metrics.padding, dpi),
  };
}

BarLayout ComputeLayout(const BarMetrics& metrics, std::size_t taskCount) {
  BarLayout layout;
  layout.startButton = {0, 0, metrics.startButtonWidth, metrics.barHeight};
  layout.showDesktop = {metrics.barWidth - metrics.showDesktopWidth, 0, metrics.barWidth,
                        metrics.barHeight};
  layout.clock = {layout.showDesktop.left - metrics.clockWidth, 0, layout.showDesktop.left,
                  metrics.barHeight};
  layout.tray = {layout.clock.left - metrics.trayWidth, 0, layout.clock.left, metrics.barHeight};

  const int areaLeft = layout.startButton.right + metrics.spacing;
  const int areaRight = layout.tray.left - metrics.spacing;
  const int area = areaRight - areaLeft;
  if (taskCount == 0 || area <= 0 || metrics.minButtonWidth <= 0) return layout;

  const int count = static_cast<int>(std::min<std::size_t>(taskCount, 4096));
  const int maxWidth = std::max(metrics.minButtonWidth, metrics.maxButtonWidth);
  const int width =
      std::clamp((area - metrics.spacing * (count - 1)) / count, metrics.minButtonWidth, maxWidth);
  const int fitting = (area + metrics.spacing) / (width + metrics.spacing);
  const int shown = std::min(count, fitting);

  const int top = metrics.padding;
  const int bottom = metrics.barHeight - metrics.padding;
  for (int i = 0; i < shown; ++i) {
    const int left = areaLeft + i * (width + metrics.spacing);
    layout.taskButtons.push_back({left, top, left + width, bottom});
  }
  return layout;
}

BarLayout ComputeVerticalLayout(const BarMetrics& metrics, int clockHeight, int trayHeight,
                                int bellHeight, std::size_t taskCount) {
  const int width = metrics.barWidth;
  const int height = metrics.barHeight;
  BarLayout layout;
  layout.startButton = {0, 0, width, width};
  layout.showDesktop = {0, height - metrics.showDesktopWidth, width, height};
  layout.notifications = {0, layout.showDesktop.top - bellHeight, width, layout.showDesktop.top};
  layout.clock = {0, layout.notifications.top - clockHeight, width, layout.notifications.top};
  layout.tray = {0, layout.clock.top - trayHeight, width, layout.clock.top};

  const int areaTop = layout.startButton.bottom + metrics.spacing;
  const int areaBottom = layout.tray.top - metrics.spacing;
  // Square buttons, as wide as the bar less its inset.
  const int buttonHeight = width - 2 * metrics.padding;
  if (taskCount == 0 || buttonHeight <= 0 || areaBottom <= areaTop) return layout;

  const int step = buttonHeight + metrics.spacing;
  const int fitting = (areaBottom - areaTop + metrics.spacing) / step;
  const int shown = static_cast<int>(std::min<std::size_t>(taskCount, std::max(fitting, 0)));
  for (int i = 0; i < shown; ++i) {
    const int top = areaTop + i * step;
    layout.taskButtons.push_back(
        {metrics.padding, top, width - metrics.padding, top + buttonHeight});
  }
  return layout;
}

Point PlacePopup(const Rect& anchor, int width, int height, Edge barEdge, int gap,
                 const Rect& screen) {
  const auto clampX = [&](int x) {
    return std::clamp(x, screen.left, std::max(screen.left, screen.right - width));
  };
  const auto clampY = [&](int y) {
    return std::clamp(y, screen.top, std::max(screen.top, screen.bottom - height));
  };
  const int centeredX = clampX((anchor.left + anchor.right - width) / 2);
  const int centeredY = clampY((anchor.top + anchor.bottom - height) / 2);
  switch (barEdge) {
    case Edge::Top:
      return {centeredX, anchor.bottom + gap};
    case Edge::Bottom:
      return {centeredX, anchor.top - gap - height};
    case Edge::Left:
      return {anchor.right + gap, centeredY};
    case Edge::Right:
      return {anchor.left - gap - width, centeredY};
  }
  return {centeredX, anchor.top - gap - height};
}

}  // namespace throwback
