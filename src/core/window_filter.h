#pragma once

namespace throwback {

// The window properties that decide whether a top-level window gets a
// taskbar button. Kept free of Win32 types so the rules can be unit tested.
struct WindowTraits {
  bool visible = false;
  bool cloaked = false;  // hidden by DWM, e.g. on another virtual desktop
  bool hasOwner = false;
  bool hasTitle = false;
  bool toolWindow = false;  // WS_EX_TOOLWINDOW
  bool appWindow = false;   // WS_EX_APPWINDOW
  bool noActivate = false;  // WS_EX_NOACTIVATE
};

// Mirrors the rules Explorer uses for its own taskbar buttons.
bool ShouldShowOnTaskbar(const WindowTraits& traits);

}  // namespace throwback
