#include "core/tray_layout.h"

#include <algorithm>
#include <cmath>

namespace throwback {
namespace {

// Windows 7 sizes at 96 DPI: icons 24 px apart, a 20 px wide arrow.
constexpr int kSlot = 24;
constexpr int kInset = 4;
constexpr int kArrowWidth = 20;
constexpr int kFlyoutPadding = 8;
constexpr int kLinkHeight = 28;
constexpr int kLinkPadding = 12;

}  // namespace

int TrayAreaWidth(std::size_t shownCount, bool hasHidden, unsigned dpi) {
  if (shownCount == 0 && !hasHidden) return 0;
  return static_cast<int>(shownCount) * ScaleForDpi(kSlot, dpi) +
         (hasHidden ? ScaleForDpi(kArrowWidth, dpi) : 0) + 2 * ScaleForDpi(kInset, dpi);
}

TrayAreaLayout ComputeTrayArea(const Rect& area, std::size_t shownCount, bool hasHidden,
                               unsigned dpi) {
  TrayAreaLayout layout;
  const int slot = ScaleForDpi(kSlot, dpi);
  int left = area.left + ScaleForDpi(kInset, dpi);
  if (hasHidden) {
    const int width = ScaleForDpi(kArrowWidth, dpi);
    layout.arrow = {left, area.top, left + width, area.bottom};
    left += width;
  }
  const int top = area.top + (area.Height() - slot) / 2;
  for (std::size_t i = 0; i < shownCount; ++i) {
    layout.icons.push_back({left, top, left + slot, top + slot});
    left += slot;
  }
  return layout;
}

TrayFlyoutLayout ComputeTrayFlyout(std::size_t count, int linkTextWidth, unsigned dpi) {
  const int slot = ScaleForDpi(kSlot, dpi);
  const int padding = ScaleForDpi(kFlyoutPadding, dpi);
  const int n = static_cast<int>(std::max<std::size_t>(count, 1));
  const int columns = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(n))));
  const int rows = (n + columns - 1) / columns;

  const int gridWidth = columns * slot;
  const int width =
      std::max(gridWidth + 2 * padding, linkTextWidth + 2 * ScaleForDpi(kLinkPadding, dpi));
  const int gridLeft = (width - gridWidth) / 2;
  const int gridBottom = padding + rows * slot + padding;

  TrayFlyoutLayout layout;
  layout.frame = {0, 0, width, gridBottom + ScaleForDpi(kLinkHeight, dpi)};
  for (int i = 0; i < static_cast<int>(count); ++i) {
    const int left = gridLeft + (i % columns) * slot;
    const int top = padding + (i / columns) * slot;
    layout.icons.push_back({left, top, left + slot, top + slot});
  }
  layout.link = {0, gridBottom, width, layout.frame.bottom};
  return layout;
}

}  // namespace throwback
