#pragma once

#include <cstddef>
#include <vector>

#include "core/layout.h"

namespace throwback {

// The notification area on the bar: the arrow for hidden icons (when there
// are any) on the left, then the icons on the bar. Sizes are pixels at the
// given DPI; area is the bar's tray rect.
struct TrayAreaLayout {
  Rect arrow;  // empty when nothing is hidden
  std::vector<Rect> icons;
};
int TrayAreaWidth(std::size_t shownCount, bool hasHidden, unsigned dpi);
TrayAreaLayout ComputeTrayArea(const Rect& area, std::size_t shownCount, bool hasHidden,
                               unsigned dpi);

// The flyout with the hidden icons: a near-square grid of them, and a strip
// with the "Customize..." link under it. Rects are relative to the flyout.
struct TrayFlyoutLayout {
  Rect frame;
  std::vector<Rect> icons;
  Rect link;
};
TrayFlyoutLayout ComputeTrayFlyout(std::size_t count, int linkTextWidth, unsigned dpi);

}  // namespace throwback
