#include "core/bar_style.h"

namespace throwback {

BarFamily BarFamilyOf(StartStyle style) {
  switch (style) {
    case StartStyle::Win98:
    case StartStyle::WinMe:
      return BarFamily::Classic98;
    case StartStyle::Win2000:
      return BarFamily::Classic2000;
    case StartStyle::WinXp:
      return BarFamily::Xp;
    case StartStyle::Vista:
      return BarFamily::Vista;
    case StartStyle::Win7:
    case StartStyle::Plain8:
      return BarFamily::Win7;
    case StartStyle::Win8:
    case StartStyle::Win81:
      return BarFamily::Win8;
    case StartStyle::Win10:
      return BarFamily::Win10;
    case StartStyle::Win11:
      break;
  }
  return BarFamily::Win11;
}

BarStyle BarStyleOf(StartStyle style) {
  BarStyle bar;
  BarMetrics& m = bar.metrics;
  m.spacing = 2;
  m.padding = 2;
  m.minButtonWidth = 64;
  m.maxButtonWidth = 160;
  switch (BarFamilyOf(style)) {
    case BarFamily::Classic98:
    case BarFamily::Classic2000:
      // A 28 px bar of 22 px buttons; the clock shows the time only.
      bar.height = 28;
      m.startButtonWidth = 54;
      m.clockWidth = 64;
      m.padding = 3;
      m.minButtonWidth = 40;
      bar.dateLine = false;
      break;
    case BarFamily::Xp:
      bar.height = 30;
      m.startButtonWidth = 99;
      m.clockWidth = 72;
      m.minButtonWidth = 40;
      bar.dateLine = false;
      break;
    case BarFamily::Vista:
      bar.height = 30;
      m.startButtonWidth = 54;
      m.clockWidth = 72;
      bar.dateLine = false;
      break;
    case BarFamily::Win7:
    case BarFamily::Win8:
      m.startButtonWidth = 56;
      m.clockWidth = 80;
      m.showDesktopWidth = 16;
      break;
    case BarFamily::Win10:
      // Combined, icon-only buttons; a sliver of Show desktop at the end.
      m.startButtonWidth = 48;
      m.clockWidth = 88;
      m.showDesktopWidth = 5;
      m.minButtonWidth = m.maxButtonWidth = 48;
      m.spacing = 0;
      m.padding = 0;
      bar.labels = false;
      break;
    case BarFamily::Win11:
      bar.height = 48;
      m.startButtonWidth = 48;
      m.clockWidth = 88;
      m.showDesktopWidth = 8;
      m.minButtonWidth = m.maxButtonWidth = 44;
      m.spacing = 4;
      m.padding = 4;
      bar.labels = false;
      break;
  }
  return bar;
}

}  // namespace throwback
