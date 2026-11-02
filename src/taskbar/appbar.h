#pragma once

#include <windows.h>

#include "core/options.h"

namespace throwback {

// Registers a window as a shell AppBar so maximized windows leave room for it.
// Unregisters itself on destruction, which gives the screen space back.
class AppBar {
 public:
  AppBar(HWND hwnd, UINT callbackMessage);
  ~AppBar();
  AppBar(const AppBar&) = delete;
  AppBar& operator=(const AppBar&) = delete;

  // Docks the window to an edge of monitor; nullptr is the primary one.
  void Dock(Edge edge, int thickness, HMONITOR monitor);
  // Re-applies the last Dock() call, e.g. after a display change.
  void Redock();
  // Handles the notifications the shell sends to callbackMessage.
  void HandleCallback(WPARAM notification, LPARAM param);
  // Drops below a fullscreen app on our monitor and comes back afterwards.
  void SetFullscreenActive(bool active);
  // Must be forwarded from WM_ACTIVATE and WM_WINDOWPOSCHANGED.
  void OnActivated();
  void OnWindowPosChanged();

 private:
  HWND hwnd_;
  Edge edge_ = Edge::Top;
  int thickness_ = 0;
  HMONITOR monitor_ = nullptr;
};

}  // namespace throwback
