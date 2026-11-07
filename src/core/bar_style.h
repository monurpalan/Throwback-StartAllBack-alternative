#pragma once

#include "core/layout.h"
#include "core/options.h"

namespace throwback {

// The taskbar looks, one per Windows generation; Windows Me shares 98's and
// 8.1 shares 8's.
enum class BarFamily { Classic98, Classic2000, Xp, Vista, Win7, Win8, Win10, Win11 };

BarFamily BarFamilyOf(StartStyle style);

// A taskbar's size and arrangement at 96 DPI, as each version had it.
struct BarStyle {
  int height = 40;
  BarMetrics metrics;    // barWidth, barHeight and trayWidth are filled at layout
  bool labels = true;    // task buttons show their window's title
  bool dateLine = true;  // the clock shows the date under the time
};

BarStyle BarStyleOf(StartStyle style);

}  // namespace throwback
