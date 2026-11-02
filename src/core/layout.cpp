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
      .minButtonWidth = ScaleForDpi(metrics.minButtonWidth, dpi),
      .maxButtonWidth = ScaleForDpi(metrics.maxButtonWidth, dpi),
      .spacing = ScaleForDpi(metrics.spacing, dpi),
      .padding = ScaleForDpi(metrics.padding, dpi),
  };
}

BarLayout ComputeLayout(const BarMetrics& metrics, std::size_t taskCount) {
  BarLayout layout;
  layout.startButton = {0, 0, metrics.startButtonWidth, metrics.barHeight};
  layout.clock = {metrics.barWidth - metrics.clockWidth, 0, metrics.barWidth, metrics.barHeight};

  const int areaLeft = layout.startButton.right + metrics.spacing;
  const int areaRight = layout.clock.left - metrics.spacing;
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

}  // namespace throwback
