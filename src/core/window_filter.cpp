#include "core/window_filter.h"

namespace throwback {

bool ShouldShowOnTaskbar(const WindowTraits& traits) {
  if (!traits.visible || traits.cloaked || traits.taskListDeleted) return false;
  // A tool window never gets a button, even with WS_EX_APPWINDOW.
  if (traits.toolWindow) return false;
  // Real store app frames have WS_EX_WINDOWEDGE; shell surfaces using the
  // same classes don't.
  if (traits.immersiveClass && !traits.windowEdge) return false;
  // WS_EX_APPWINDOW forces a button for owned and no-activate windows.
  if (traits.appWindow) return true;
  return !traits.hasOwner && !traits.noActivate;
}

}  // namespace throwback
