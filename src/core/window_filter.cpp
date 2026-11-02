#include "core/window_filter.h"

namespace throwback {

bool ShouldShowOnTaskbar(const WindowTraits& traits) {
  if (!traits.visible || traits.cloaked) return false;
  // WS_EX_APPWINDOW forces a button even for owned or tool windows.
  if (traits.appWindow) return true;
  if (traits.toolWindow || traits.hasOwner || traits.noActivate) return false;
  return traits.hasTitle;
}

}  // namespace throwback
